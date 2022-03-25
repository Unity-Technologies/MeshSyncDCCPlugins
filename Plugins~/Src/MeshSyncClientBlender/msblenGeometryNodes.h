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
		static mu::float4x4 blenderToUnityWorldMatrix(mu::float4x4& blenderMatrix);

		static mu::float4x4 blenderToUnityWorldMatrixMesh();

		/// <summary>
		/// Invokes the handler function for each instance.
		/// </summary>
		/// <param name="handler">
		/// The handling function: 
		/// instancedObject is the object that is being instanced.
		/// transform is the transform of the instance
		/// </param>
		static void foreach_instance(std::function<void (Object*, Object*, mu::float4x4)> handler);

		/// <summary>
		/// Invokes the handler function for each instanced object.
		/// </summary>
		/// <param name="handler">
		/// The handling function: 
		/// instancedObject is the object that is being instanced.
		/// parent is the object that has the geometry node modifier.
		/// transforms is the collection of transforms for the instanced object.
		/// </param>
		static void foreach_instanced_object(std::function<void(Object*, Object*, SharedVector<mu::float4x4>)> handler);

		void setInstancesDirty(bool dirty);
		bool getInstancesDirty();

	private:
		bool m_instances_dirty;

		struct Record {
			Object* obj = nullptr;
			Object* parent = nullptr;
			SharedVector<mu::float4x4> matrices;
			bool handled = false;
		};
	};
#endif
}

