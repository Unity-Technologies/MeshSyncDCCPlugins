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
        /// <summary>
        /// /// Converts the world matrix from blender to Unity coordinate system
        /// /// </summary>
		static mu::float4x4& blenderToUnityWorldMatrix(mu::float4x4& blenderMatrix);

		static mu::float4x4& blenderToUnityWorldMatrixMesh();

		/// <summary>
		/// Will invoke f for every instance. The first argument of f is the name
		/// of the mesh that is being instantiated and the second argumenet is the world matrix
		/// of the instance in the Unity3D Coordinate system
		/// </summary>
		/// <param name="f"></param>
		static void foreach_instance(
			std::function<void (std:: string, mu::float4x4)> pathHandler, 
			std::function<void(Mesh*, mu::float4x4)> meshHandler);

		static void foreach_instance(
			std::function<void(std::string, std::vector<mu::float4x4>)> pathHandler, 
			std::function<void(Mesh*, std::vector<mu::float4x4>)> meshHandler);

		void setInstancesDirty(bool dirty);
		bool getInstancesDirty();

	private:
		bool m_instances_dirty;
	};
#endif
}

