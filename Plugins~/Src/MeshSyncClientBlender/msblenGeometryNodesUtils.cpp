#include "msblenGeometryNodeUtils.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <BlenderPyObjects/BlenderPyDepsgraphObjectInstance.h>
#include <BlenderPyObjects/BlenderPyDepsgraph.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyScene.h"
#include <msblenUtils.h>
#include <BLI_listbase.h>


using namespace std;
using namespace mu;

namespace blender {

#if BLENDER_VERSION >= 300
    GeometryNodesUtils::GeometryNodesUtils()
    {
        auto rotation = rotate_x(-90 * DegToRad);
        auto rotation180 = rotate_z(180 * DegToRad);
        auto scale_z = float3::one();
        scale_z.z = -1;

        auto scale_x = float3::one();
        scale_x.x = -1;

        m_blender_to_unity_world =
            to_mat4x4(rotation) *
            scale44(scale_x);

        m_blender_to_unity_local = 
            to_mat4x4(rotation) *
            to_mat4x4(rotation180) *
            scale44(scale_z);
    }

    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate systems
    /// </summary>
    /// <param name="blenderMatrix"></param>
    /// <returns></returns>
    float4x4 GeometryNodesUtils::blenderToUnityWorldMatrix(const float4x4& blenderMatrix) {            

        return 
            m_blender_to_unity_world *
            blenderMatrix *
            m_blender_to_unity_local;
    }

    void GeometryNodesUtils::setInstancesDirty(bool dirty)
    {
        m_instances_dirty = dirty;
    }
    bool GeometryNodesUtils::getInstancesDirty()
    {
        return m_instances_dirty;
    }

    void GeometryNodesUtils::each_instanced_object(std::function<void(Object*, Object*, SharedVector<float4x4>, bool)> handler) {

        std::unordered_map<unsigned int, Record> records;
        std::unordered_map<std::string, Record*> records_by_name;

        each_instance([&](Object* obj, Object* parent, float4x4 matrix)
            {
                // Critical path, must do as few things as possible
                auto id = (ID*)obj->data;
                auto& rec = records[id->session_uuid];
                if (!rec.updated)
                {
                    rec.parent = parent;
                    std::memcpy(&rec.object_copy, obj, sizeof(Object));
                    rec.updated = true;
    
                    records_by_name[obj->id.name] = &rec;
                }

                rec.matrices.push_back(matrix);
            });

            // Look for objects in the file
            auto ctx = blender::BlenderPyContext::get();
            auto objects = ctx.data()->objects;
            LISTBASE_FOREACH(Object*, obj, &objects) {

                if (obj->data == nullptr)
                    continue;

                // Check if there is record with the same object name
                auto rec = records_by_name.find(obj->id.name);
                if (rec == records_by_name.end())
                    continue;

                // Sometimes an object in the data might come up more than once
                if (rec->second->handled)
                    continue;

                // Check if the data names also match
                auto recDataId = (ID*)rec->second->object_copy.data;
                auto sceneDataId = (ID*)obj->data;

                if (strcmp(sceneDataId->name + 2, recDataId->name + 2) != 0)
                    continue;

                handler(obj, rec->second->parent, std::move(rec->second->matrices), true);
                rec->second->handled = true;

            }

            // Export objects that are not in the file
            for (auto& rec : records) {
                if (rec.second.handled)
                    continue;


                handler(&rec.second.object_copy, rec.second.parent, std::move(rec.second.matrices), false);
                rec.second.handled = true;
            }

    }

    void GeometryNodesUtils::each_instance(std::function<void(Object*, Object*, float4x4)> handler)
    {
        auto blender_ctx = BlenderPyContext::get();
        auto depsgraph_ctx = blender_ctx.evaluated_depsgraph_get();

        auto depsgraph = BlenderPyDepsgraph(depsgraph_ctx);
        

        // Iterate over the object instances collection of depsgraph
        CollectionPropertyIterator it;

        depsgraph.object_instances_begin(&it);

        for (; it.valid; depsgraph.object_instances_next(&it)) {

            // Get the instance as a Pointer RNA.
            auto instance_ctx = depsgraph.object_instances_get(&it);
            auto instance = BlenderPyDepsgraphInstance(instance_ctx);

            // Get the object that the instance refers to
            auto instance_object = instance.instance_object();

            // If the object is null, skip
            if (instance_object == nullptr)
                continue;

            // if the instance is not an instance, skip
            if (!instance.is_instance())
                continue;

            auto object = instance.object();

            // We support only Mesh and Light instances
            if (object->type != OB_MESH && 
                object->type != OB_LAMP) {
                continue;
            }

            auto world_matrix = float4x4();
            instance.world_matrix(&world_matrix);

            auto parent = instance.parent();

            handler(object, parent, world_matrix);
        }

        // Cleanup resources
        depsgraph.object_instances_end(&it);
    }
#endif
}
