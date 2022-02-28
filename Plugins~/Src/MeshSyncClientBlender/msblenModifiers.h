#pragma once
#include <unordered_map>
#include "pch.h"
#include "MeshUtils/muMath.h"
#include "DNA_object_types.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include <DNA_node_types.h>

namespace blender {

	class msblenModifiers
	{
	private:
		bool addModifierProperties(ms::TransformPtr transform, ModifierData* modifier, std::stringstream& names);

	public:		
		void exportModifiers(ms::TransformPtr transform, const Object* obj);
	};
}