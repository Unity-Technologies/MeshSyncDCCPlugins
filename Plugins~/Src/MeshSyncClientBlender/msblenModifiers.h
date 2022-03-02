#pragma once
#include <DNA_node_types.h>
#include <MeshSyncClient/msPropertyManager.h>

namespace blender {
	class msblenModifiers
	{
	public:
		static void exportModifiers(ms::TransformPtr transform, const Object* obj, ms::PropertyManager* propertyManager);
	};
}
