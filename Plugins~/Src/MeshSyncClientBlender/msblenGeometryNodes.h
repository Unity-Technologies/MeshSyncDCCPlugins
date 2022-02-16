#pragma once
#include <unordered_map>
#include "pch.h"
#include "MeshUtils/muMath.h"
#include "DNA_object_types.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include <DNA_node_types.h>

namespace blender {

	class msblenGeometryNodes
	{
		private:
			// Object instances
			typedef std::vector<mu::float4x4> matrix_vector;
			typedef std::unordered_map<std::string, matrix_vector> object_instances_t;
			object_instances_t instances;
			void findObjectInstances();
			void clearObjectInstances();
			void addInstanceData(const Object* src, ms::Mesh& dst);

		public:
			// Blender Application events
			void onDepsgraphUpdatePost(Depsgraph* graph);

			// MeshSync events
			void onExportComplete();
			void onMeshExport(const Object* obj, ms::Mesh& mesh);
	};
}

