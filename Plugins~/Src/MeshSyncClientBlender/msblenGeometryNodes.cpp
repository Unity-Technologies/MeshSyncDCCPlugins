#include "msblenGeometryNodes.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyNodeTree.h"
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
    float4x4 GeometryNodesUtils::blenderToUnityWorldMatrix(float4x4& blenderMatrix) {            

        return 
            m_blender_to_unity_world *
            blenderMatrix *
            m_blender_to_unity_local;;
    }

    void GeometryNodesUtils::foreach_instance(std::function<void(Object*, Object* , float4x4)> handler)
    {

        auto blContext = blender::BlenderPyContext::get();


        // BlenderPyContext is an interface between depsgraph operations and anything that interacts with it
        auto depsgraph = blContext.evaluated_depsgraph_get();

        // Iterate over the object instances collection of depsgraph
        CollectionPropertyIterator it;

        blContext.object_instances_begin(&it, depsgraph);

        for (; it.valid; blContext.object_instances_next(&it)) {
            // Get the instance as a Pointer RNA.
            auto instance = blContext.object_instances_get(&it);

            // Get the object that the instance refers to
            auto instance_object = blContext.instance_object_get(instance);


            // If the object is null, skip
            if (instance_object == nullptr)
                continue;

            // if the instance is not an instance, skip
            if (!blContext.object_instances_is_instance(instance))
                continue;

            auto object = blContext.object_get(instance);

            if (object->type != OB_MESH) {
                continue;
            }

            auto world_matrix = float4x4();
            blContext.world_matrix_get(&instance, &world_matrix);

            auto unityMatrix = blenderToUnityWorldMatrix(world_matrix);
            auto parent = blContext.instance_parent_get(&instance);

            handler(object, parent, move(unityMatrix));
        }

        // Cleanup resources
        blContext.object_instances_end(&it);
    }

    void GeometryNodesUtils::foreach_instanced_object(function<void(Object*, Object*, SharedVector<float4x4>, bool)> handler) {
        
        m_records.clear();
        m_records_by_name.clear();

        foreach_instance([&](Object* obj, Object* parent, float4x4 matrix) {
            // Critical path, must do as few things as possible
            auto id = (ID*)obj->data;
            auto& rec = m_records[id->session_uuid];
            
            if (!rec.updated) {
                rec.parent = parent;
                rec.object_copy = *obj;
                rec.updated = true;

                m_records_by_name[obj->id.name] = &rec;
            }
            
            rec.matrices.push_back(matrix);
            });

        // Look for objects in the file
        auto ctx = blender::BlenderPyContext::get();
        auto objects = ctx.data()->objects;
        LISTBASE_FOREACH(Object*, obj, &objects){

            if (obj->data == nullptr)
                continue;

            // Check if there is record with the same object name
            auto rec = m_records_by_name.find(obj->id.name);
            if (rec == m_records_by_name.end())
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
        for(auto& rec : m_records) {
            if (rec.second.handled)
                continue;
            
            handler(&rec.second.object_copy, rec.second.parent, std::move(rec.second.matrices), false);
            rec.second.handled= true;
        }
    }

    void GeometryNodesUtils::setInstancesDirty(bool dirty)
    {
        m_instances_dirty = dirty;
    }
    bool GeometryNodesUtils::getInstancesDirty()
    {
        return m_instances_dirty;
    }

    void blender::GeometryNodesUtils::clear()
    {
        m_records.clear();
        m_records_by_name.clear();
    }
#endif
}
