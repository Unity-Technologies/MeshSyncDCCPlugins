#pragma once
#include <DNA_node_types.h>
#include <MeshSyncClient/msPropertyManager.h>
#include "MeshSync/SceneGraph/msPropertyInfo.h"

namespace blender {
	class msblenModifiers
	{
	public:
		static void exportProperties(const Object* obj, ms::PropertyManager* propertyManager);

		static void importProperties(std::vector<ms::PropertyInfo> props);
	};
}
