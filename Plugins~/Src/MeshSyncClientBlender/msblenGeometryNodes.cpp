#include "msblenGeometryNodes.h"
#include "BlenderPyObjects/BlenderPyDepsgraphUpdate.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyNodeTree.h"


using namespace std;
using namespace mu;

namespace blender {

    // UTILS

    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate systems
    /// </summary>
    /// <param name="blenderMatrix"></param>
    /// <returns></returns>
    float4x4& blenderToUnityWorldMatrix(float4x4& blenderMatrix) {

        auto rotation = rotate_x(-90 * DegToRad);
        auto rotation180 = rotate_z(180 * DegToRad);
        auto scale = float3::one();
        scale.x = -1;

        auto result =
            to_mat4x4(rotation) *
            scale44(scale) *
            blenderMatrix *
            to_mat4x4(rotation) *
            to_mat4x4(rotation180) *
            scale44(scale);

        return move(result);
    }

    // INSTANCES

    void msblenGeometryNodes::findObjectInstances() {

        using namespace std;
        using namespace mu;

        instances.clear();

        // BlenderPyContext is an interface between depsgraph operations and anything that interacts with it
        auto blContext = blender::BlenderPyContext::get();
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

            auto object_data = (ID*)object->data;
            if (object_data == nullptr)
                continue;

            auto name = object_data->name;

            if (name == nullptr)
                continue;

            if (instances.find(name) == instances.end()) {
                matrix_vector v;
                instances.insert(move(pair(name, move(v))));
            }

            auto world_matrix = float4x4();
            blContext.world_matrix_get(&instance, &world_matrix);

            auto result = blenderToUnityWorldMatrix(world_matrix);

            instances[name].push_back(move(result));
        }

        // Cleanup resources
        blContext.object_instances_end(&it);
    }

    void msblenGeometryNodes::clearObjectInstances()
    {
        for (auto it = instances.begin(); it != instances.end(); it++) {
            auto data = *it;
            data.second.clear();
        }
    }

    void msblenGeometryNodes::addInstanceData(const Object* src, ms::Mesh& dst)
    {
        auto data = (ID*)src->data;

        if (data == nullptr)
            return;

        auto meshName = data->name;

        auto meshInstances = instances.find(meshName);
        if (instances.find(meshName) == instances.end())
            return;

        if (meshInstances->second.size() == 0)
            return;

        /// Add mesh instances as a user property
        if (dst.findUserProperty("instances")== nullptr){

            auto matrices = instances[meshName];

            ms::Variant newProperty;

            newProperty.name = "instances";
            newProperty.type = ms::Variant::Type::Float4x4;
            newProperty.set(matrices.data(), matrices.size());
            dst.addUserProperty(move(newProperty));
        }
    }

    // EVENTS

    void msblenGeometryNodes::onDepsgraphUpdatePost(Depsgraph* graph)
    {
        findObjectInstances();
    }

    void msblenGeometryNodes::onExportComplete()
    {
        clearObjectInstances();
    }

    void msblenGeometryNodes::onMeshExport(const Object* obj, ms::Mesh& mesh)
    {
        addInstanceData(obj, mesh);
    }
}
