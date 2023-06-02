#pragma once
#include <unordered_map>
#include "pch.h"
#include "MeshUtils/muMath.h"
#include "DNA_object_types.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include <DNA_node_types.h>
#include "MeshSyncClient/msInstancesManager.h"
#include "MeshUtils/muMath.h"
#include <DNA_mesh_types.h>

namespace blender {

#if BLENDER_VERSION >= 300
class GeometryNodesUtils
{
public:

    GeometryNodesUtils();

    struct Record {
        Object* parent = nullptr;
        Object* obj = nullptr;
        SharedVector<mu::float4x4> matrices;
        bool handled_matrices = false;
        bool handled_object = false;
        bool from_file = false;
        std::string name;
        std::string id;
    };

    /// <summary>
    /// Invokes the handler function for each instance.
    /// </summary>
    /// <param name="handler">
    /// The handling function: 
    /// instancedObject is the object that is being instanced.
    /// transform is the transform of the instance
    /// </param>
    void each_instance(std::function<void(Object*, Object*, mu::float4x4)> handler);

    /// <summary>
    /// Invokes the handler function for each instanced object.
    /// </summary>
    /// <param name="handler">
    /// The handling function: 
    /// instancedObject is the object that is being instanced.
    /// parent is the object that has the geometry node modifier.
    /// transforms is the collection of transforms for the instanced object.
    /// </param>
    void each_instanced_object(
        std::function<void(GeometryNodesUtils::Record& rec)> object_handler,
        std::function<void(GeometryNodesUtils::Record& rec)> matrix_handler);

    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate system
    /// </summary>
    mu::float4x4 blenderToUnityWorldMatrix(ms::TransformPtr transform, const mu::float4x4& blenderMatrix) const;
    
    void setInstancesDirty(bool dirty);
    bool getInstancesDirty();

    std::string get_data_path(Object* obj);

private:
    bool m_instances_dirty;

    mu::float4x4 m_blender_to_unity_local;
    mu::float4x4 m_blender_to_unity_world;
    mu::float4x4 m_camera_light_correction;
};


#endif
}

