#include "msblenModifiers.h"
#include "BlenderPyObjects/BlenderPyDepsgraphUpdate.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyNodeTree.h"
#include <msblenBinder.h>
#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include <msblenUtils.h>
#include "BlenderPyObjects/BlenderPyContext.h"
#include "msblenBinder.h"

namespace blender {
#if BLENDER_VERSION < 300
	void msblenModifiers::exportModifiers(ms::TransformPtr transform, const Object* obj, ms::PropertyManager* propertyManager) {}
	void msblenModifiers::importModifiers(std::vector<ms::PropertyInfo> props) {}
#else

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


	std::mutex m_mutex;

	bNodeSocket* getSocketForProperty(IDProperty* property, bNodeTree* group, BlenderPyNodeTree blNodeTree) {
		CollectionPropertyIterator it;
		for (blNodeTree.inputs_begin(&it, group); it.valid; blNodeTree.inputs_next(&it)) {
			auto input = blNodeTree.inputs_get(&it);
			auto socket = (bNodeSocket*)input.data;
			if (strcmp(socket->identifier, property->name) == 0) {
				return socket;
			}
		}

		return nullptr;
	}

	bool doesPropertyUseAttribute(std::string propertyName, NodesModifierData* nodeModifier) {
		auto attributeName = propertyName + "_use_attribute";

		// Loop through modifier data and get the values:
		LISTBASE_FOREACH(IDProperty*, property, &nodeModifier->settings.properties->data.group) {
			if (property->name == attributeName) {
				return IDP_Int(property);
			}
		}

		return false;
	}

	void addModifierProperties(ms::TransformPtr transform, ModifierData* modifier, const Object* obj, ms::PropertyManager* propertyManager)
	{
		if (modifier->type != ModifierType::eModifierType_Nodes) {
			return;
		}

		auto blNodeTree = blender::BlenderPyNodeTree();
		auto nodeModifier = (NodesModifierData*)modifier;
		auto group = nodeModifier->node_group;

		// Loop through modifier data and get the values:
		LISTBASE_FOREACH(IDProperty*, property, &nodeModifier->settings.properties->data.group) {
			if (strstr(property->name, "_use_attribute") || strstr(property->name, "_attribute_name")) {
				continue;
			}

			if (doesPropertyUseAttribute(property->name, nodeModifier)) {
				continue;
			}

			auto socket = getSocketForProperty(property, group, blNodeTree);

			if (socket != nullptr) {
				auto propertyInfo = ms::PropertyInfo::create();

				switch (property->type) {
				case IDP_INT: {
					auto defaultValue = (bNodeSocketValueInt*)socket->default_value;
					propertyInfo->set(IDP_Int(property), defaultValue->min, defaultValue->max);
					break;
				}
				case IDP_FLOAT: {
					auto defaultValue = (bNodeSocketValueFloat*)socket->default_value;
					propertyInfo->set(IDP_Float(property), defaultValue->min, defaultValue->max);
					break;
				}
				case IDP_ARRAY: {
					auto defaultValue = (bNodeSocketValueVector*)socket->default_value;
					switch (property->subtype) {
					case IDP_INT: {
						propertyInfo->set((int*)IDP_Array(property), defaultValue->min, defaultValue->max, property->len);
						break;
					}
					case IDP_FLOAT: {
						propertyInfo->set((float*)IDP_Array(property), defaultValue->min, defaultValue->max, property->len);
						break;
					}
					}
					break;
				}
				default:
					continue;
				}

				propertyInfo->path = get_path(obj);
				propertyInfo->name = socket->name;
				propertyInfo->modifierName = modifier->name;
				propertyInfo->propertyName = property->name;
				propertyManager->add(propertyInfo);
			}
		}
	}

	void msblenModifiers::exportModifiers(ms::TransformPtr transform, const Object* obj, ms::PropertyManager* propertyManager)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		blender::BObject bObj(obj);
		auto modifiers = bObj.modifiers();
		for (auto it = modifiers.begin(); it != modifiers.end(); ++it) {

			auto modifier = *it;

			// Add each modifier as a variant
			addModifierProperties(transform, modifier, obj, propertyManager);
		}
	}

	void msblenModifiers::importModifiers(std::vector<ms::PropertyInfo> props) {
		if (props.size() == 0) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_mutex);
		// Apply returned properties:
		for (auto& receivedProp : props) {
			auto obj = get_object_from_path(receivedProp.path);

			if (!obj) {
				continue;
			}

			auto modifier = FindModifier(obj, receivedProp.modifierName);

			auto nodeModifier = (NodesModifierData*)modifier;

			LISTBASE_FOREACH(IDProperty*, property, &nodeModifier->settings.properties->data.group) {
				if (property->name == receivedProp.propertyName) {

					switch (receivedProp.type) {
					case ms::PropertyInfo::Type::Int: {
						auto val = receivedProp.get<int>();
						IDP_Int(property) = val;
						break;
					}
					case ms::PropertyInfo::Type::Float: {
						auto val = receivedProp.get<float>();
						IDP_Float(property) = val;
						break;
					}
					case ms::PropertyInfo::Type::FloatArray:
					case ms::PropertyInfo::Type::IntArray:
						receivedProp.copy(IDP_Array(property));
						break;
					default:
						break;
					}

					switch (obj->type) {
					case OB_MESH:
					{
						auto mesh = (BMesh*)obj->data;
						BMesh(mesh).update();
						break;
					}
					}
				}
			}
		}

		auto pyContext = blender::BlenderPyContext::get();
		auto depsGraph = pyContext.evaluated_depsgraph_get();
		BlenderPyContext::UpdateDepsgraph(depsGraph);
	}

#endif // BLENDER_VERSION < 300

} // namespace blender 
