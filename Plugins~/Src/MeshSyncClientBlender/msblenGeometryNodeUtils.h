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
    void each_instanced_object(std::function<void(Object*, Object*, SharedVector<mu::float4x4>, bool)> handler);

    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate system
    /// </summary>
    mu::float4x4 blenderToUnityWorldMatrix(const Object* obj, const mu::float4x4& blenderMatrix);
    
    void setInstancesDirty(bool dirty);
    bool getInstancesDirty();

private:
    bool m_instances_dirty;

    mu::float4x4 m_blender_to_unity_local;
    mu::float4x4 m_blender_to_unity_world;
    mu::float4x4 m_camera_light_correction;
        
    struct Record {
        Object object_copy;
        Object* parent = nullptr;
        SharedVector<mu::float4x4> matrices;
        bool handled = false;
        bool updated = false;
    };
};


#endif
}

