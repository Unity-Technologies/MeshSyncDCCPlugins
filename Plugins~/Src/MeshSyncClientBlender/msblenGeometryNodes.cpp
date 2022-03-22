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
    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate systems
    /// </summary>
    /// <param name="blenderMatrix"></param>
    /// <returns></returns>
    float4x4 GeometryNodesUtils::blenderToUnityWorldMatrix(float4x4& blenderMatrix) {

        auto rotation = rotate_x(-90 * DegToRad);
        auto rotation180 = rotate_z(180 * DegToRad);
        auto scale_z = float3::one();
        scale_z.z = -1;

        auto scale_x = float3::one();
        scale_x.x = -1;

        auto result =
            to_mat4x4(rotation) *
            scale44(scale_x) *
            blenderMatrix *
            to_mat4x4(rotation) *
            to_mat4x4(rotation180) *
            scale44(scale_z);

        return result;
    }

    mu::float4x4 GeometryNodesUtils::blenderToUnityWorldMatrixMesh()
    {
        auto rotation = rotate_x(90.0f * DegToRad);
        auto scale = float3::one();
        scale.x = -1;

        auto result =
            to_mat4x4(rotation)*
            scale44(scale);

        return result;
    }


    void GeometryNodesUtils::foreach_instance(std::function<void(ID*, float4x4)> handler)
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

            handler(&object->id, move(unityMatrix));
        }

        // Cleanup resources
        blContext.object_instances_end(&it);
    }

    void GeometryNodesUtils::foreach_instance(function<void(Object*, SharedVector<float4x4>)> handler) {

        map<string, SharedVector<float4x4>> transformsMap;
        foreach_instance([&](ID* id, float4x4 matrix) {
            transformsMap[id->name].push_back(matrix);
            });

        auto ctx = blender::BlenderPyContext::get();

        auto objects = ctx.data()->objects;
        LISTBASE_FOREACH(Object*, obj, &objects){
            auto result = transformsMap.find(obj->id.name);
            if (result != transformsMap.end()) {
                handler(obj, std::move(result->second));
            }
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
#endif
}
