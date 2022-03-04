#include "msblenModifiers.h"
#include "BlenderPyObjects/BlenderPyDepsgraphUpdate.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyNodeTree.h"
#include <msblenBinder.h>
#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include <msblenUtils.h>

// Copied from blender source that we cannot include:
#define LISTBASE_FOREACH(type, var, list) \
  for (type var = (type)((list)->first); var != NULL; var = (type)(((Link *)(var))->next))

#define IDP_Int(prop) ((prop)->data.val)
#define IDP_Array(prop) ((prop)->data.pointer)
#define IDP_Float(prop) (*(float *)&(prop)->data.val)
#define IDP_Double(prop) (*(double *)&(prop)->data.val)
#define IDP_String(prop) ((char *)(prop)->data.pointer)
#define IDP_IDPArray(prop) ((struct IDProperty *)(prop)->data.pointer)
#define IDP_Id(prop) ((ID *)(prop)->data.pointer)


namespace blender {

	bNodeSocket* getSocketForProperty(IDProperty* property, bNodeTree* group, BlenderPyNodeTree blNodeTree) {
		CollectionPropertyIterator it;
		//blNodeTree.inputs_begin(&it, group);
		for (blNodeTree.inputs_begin(&it, group); it.valid; blNodeTree.inputs_next(&it)) {
			auto input = blNodeTree.inputs_get(&it);
			auto socket = (bNodeSocket*)input.data;
			if (strcmp(socket->identifier, property->name) == 0) {
				return socket;
			}
		}

		return nullptr;
	}

	//void addUserProperty(std::string name, IDProperty* property, ms::TransformPtr transform) {
	//	auto variant = ms::Variant();
	//	variant.name = name;

	//	switch (property->type) {
	//	case IDP_INT: {
	//		auto val = IDP_Int(property);

	//		variant.type = ms::Variant::Type::Int;
	//		//variant.set(std::move(val));
	//		variant.set(val);
	//		break;
	//	}
	//	case IDP_FLOAT: {
	//		auto val = IDP_Float(property);

	//		variant.type = ms::Variant::Type::Float;
	//		variant.set(val);
	//		break;
	//	}
	//	default:
	//		return;
	//	}

	//	transform->addUserProperty(std::move(variant));
	//}

	bool addModifierProperties(ms::TransformPtr transform, ModifierData* modifier, std::stringstream& names, const Object* obj, ms::PropertyManager* propertyManager)
	{
#if BLENDER_VERSION >= 300
		if (modifier->type != ModifierType::eModifierType_Nodes) {
			return false;
		}

		auto blNodeTree = blender::BlenderPyNodeTree();
		auto nodeModifier = (NodesModifierData*)modifier;
		auto group = nodeModifier->node_group;

		// Loop through modifier data and get the values:
		LISTBASE_FOREACH(IDProperty*, property, &nodeModifier->settings.properties->data.group) {
			if (strstr(property->name, "_use_attribute") || strstr(property->name, "_attribute_name")) {
				continue;
			}

			auto socket = getSocketForProperty(property, group, blNodeTree);

			if (socket != nullptr) {
				//auto variant = ms::Variant();
				// variant.name = socket->name;
				auto propertyInfo = ms::PropertyInfo::create();

				switch (property->type) {
				case IDP_INT: {
					auto defaultValue = (bNodeSocketValueInt*)socket->default_value;
					propertyInfo->set(IDP_Int(property), defaultValue->min, defaultValue->max);
					//variant.type = ms::Variant::Type::Int;
					//variant.set(std::move(val));
					//variant.set(val);
					break;
				}
				case IDP_FLOAT: {
					/*auto val = IDP_Float(property);

					variant.type = ms::Variant::Type::Float;
					variant.set(val);*/

					auto defaultValue = (bNodeSocketValueFloat*)socket->default_value;
					propertyInfo->set(IDP_Float(property), defaultValue->min, defaultValue->max);
					break;
				}
				default:
					continue;
				}

				propertyInfo->path = get_path(obj);
				propertyInfo->name = socket->name;
				propertyInfo->modifierName = modifier->name;
				propertyManager->add(propertyInfo);

				//names << variant.name << std::endl;
				//transform->addUserProperty(std::move(variant));

				//if (variant.type != ms::Variant::Type::Unknown) {
				//	names << variant.name << std::endl;

				//	switch (variant.type)
				//	{
				//	case ms::Variant::Type::Int:
				//	{
				//		auto defaultValue = (bNodeSocketValueInt*)socket->default_value;
				//		variant.set(std::move(defaultValue->min));

				//		break;
				//	}
				//	default:
				//		break;
				//	}
				//}
			}
		}

		return true;
#else
		return false;
#endif // BLENDER_VERSION >= 300
	}

	void msblenModifiers::exportModifiers(ms::TransformPtr transform, const Object* obj, ms::PropertyManager* propertyManager)
	{
		// Add the geometry node properties as a user property

		// Create a manifest with the names of the modifiers
		std::stringstream modifierNames;
		blender::BObject bObj(obj);
		auto modifiers = bObj.modifiers();
		for (auto it = modifiers.begin(); it != modifiers.end(); ++it) {

			auto modifier = *it;

			// Add each modifier as a variant
			addModifierProperties(transform, modifier, modifierNames, obj, propertyManager);
		}

		auto streamString = modifierNames.str();
		if (!streamString.empty()) {
			ms::Variant modifierManifest;
			modifierManifest.name = "modifiers";
			modifierManifest.type = ms::Variant::Type::String;

			modifierManifest.set(streamString.c_str(), streamString.length());

			transform->addUserProperty(std::move(modifierManifest));
		}
	}

	void msblenModifiers::applyModifiers(std::vector<ms::PropertyInfo> props) {
		// Apply returned properties:
		for (auto& prop : props) {
			prop.path
		}
	}

} // namespace blender 