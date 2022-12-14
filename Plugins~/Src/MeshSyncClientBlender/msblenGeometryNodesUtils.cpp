#include "msblenGeometryNodeUtils.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <BlenderPyObjects/BlenderPyDepsgraphObjectInstance.h>
#include <BlenderPyObjects/BlenderPyDepsgraph.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyScene.h"
#include <msblenUtils.h>
#include <BLI_listbase.h>

#include "msblenEntityHandler.h"


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
        
        m_camera_light_correction = to_mat4x4(rotate_x(90 * DegToRad));
    }

    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate systems
    /// </summary>
    /// <param name="blenderMatrix"></param>
    /// <returns></returns>
    float4x4 GeometryNodesUtils::blenderToUnityWorldMatrix(ms::TransformPtr transform, const float4x4& blenderMatrix) const
    {
        float4x4 result = blenderMatrix;

        auto type = transform->getType();
        auto is_camera = type == ms::Entity::Type::Camera;
        auto is_light = type == ms::Entity::Type::Light;
        msblenEntityHandler::applyCorrectionIfNeeded(result, is_camera, is_light);

        result = m_blender_to_unity_world *
            result *
            m_blender_to_unity_local;

        // Apply inverse of the correction because the original of the instanced light/camera
        // would have the correction applied and without this its inverse would not match
        // the inverse we use on blender's side:
        if (is_camera || is_light) {
            result = m_camera_light_correction * result;
        }
        
        return result;
    }
        
    void GeometryNodesUtils::setInstancesDirty(bool dirty)
    {
        m_instances_dirty = dirty;
    }
    bool GeometryNodesUtils::getInstancesDirty()
    {
        return m_instances_dirty;
    }

    void GeometryNodesUtils::each_instanced_object(
        std::function<void(Record&)> obj_handler,
        std::function<void(Record&)> matrix_handler) {

        std::unordered_map<unsigned int, Record> records;

        // Collect object names in the file
        auto ctx = blender::BlenderPyContext::get();
        auto objects = ctx.data()->objects;
        std::unordered_map<std::string, Object*> file_objects;
        LISTBASE_FOREACH(Object*, obj, &objects) {

            if (obj->data == nullptr)
                continue;

            auto id = (ID*)obj->data;

            file_objects[id->name + 2] = obj;
        }

        each_instance([&](Object* obj, Object* parent, float4x4 matrix)
            {
                auto id = (ID*)obj->data;
                auto& rec = records[id->session_uuid];
                if (!rec.handled_object)
                {
                    rec.handled_object = true;
                    rec.name = id->name + 2;
                    rec.obj = obj;
                    rec.parent = parent;
                    rec.from_file = file_objects.find(rec.name) != file_objects.end();
                    rec.id = id->session_uuid;
                    obj_handler(rec);
                }
                
                rec.matrices.push_back(matrix);
            });



            // Export transforms
            for (auto& rec : records) {
                if (rec.second.handled_matrices)
                    continue;

                rec.second.handled_matrices = true;
                matrix_handler(rec.second);
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

            // Don't instance empties, they have no data we can use to get a session id:
            if (object->type == OB_EMPTY) {
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
