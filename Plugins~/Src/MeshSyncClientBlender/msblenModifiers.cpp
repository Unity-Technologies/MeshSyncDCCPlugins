#include "msblenModifiers.h"
#include <sstream>
#include <MeshSync/SceneGraph/msMesh.h>
#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include <msblenUtils.h>
#include "BlenderPyObjects/BlenderPyContext.h"
#include "msblenBinder.h"

namespace blender {
#if BLENDER_VERSION < 300
void msblenModifiers::exportProperties(const Object* obj, ms::TransformPtr dst, ms::PropertyManager* propertyManager, msblenContextPathProvider& paths) {}
void msblenModifiers::importProperties(std::vector<ms::PropertyInfo> props) {}
bool msblenModifiers::doesObjectHaveCustomProperties(const Object* obj) { return false; }
#else

// Copied from blender source that we cannot include:
#define IDP_Int(prop) ((prop)->data.val)
#define IDP_Array(prop) ((prop)->data.pointer)
#define IDP_Float(prop) (*(float *)&(prop)->data.val)
#define IDP_Double(prop) (*(double *)&(prop)->data.val)
#define IDP_String(prop) ((char *)(prop)->data.pointer)
#define IDP_IDPArray(prop) ((struct IDProperty *)(prop)->data.pointer)
#define IDP_Id(prop) ((ID *)(prop)->data.pointer)

std::mutex m_mutex;

bNodeSocket* getSocketForProperty(IDProperty* property, bNodeTree* group) {
	for (bNodeSocket* socket : blender::list_range((bNodeSocket*)group->inputs.first)) {
		if (strcmp(socket->identifier, property->name) == 0) {
			return socket;
		}
	}

	return nullptr;
}

bool doesPropertyUseAttribute(std::string propertyName, NodesModifierData* nodeModifier) {
	auto attributeName = propertyName + "_use_attribute";

	// Loop through modifier data and get the values:
	for (auto property : blender::list_range((IDProperty*)nodeModifier->settings.properties->data.group.first)) {
		if (property->name == attributeName) {
			return IDP_Int(property);
		}
	}

	return false;
}

void addModifierProperties(ModifierData* modifier, const Object* obj, ms::PropertyManager* propertyManager)
{
	if (modifier->type != ModifierType::eModifierType_Nodes) {
		return;
	}

	auto nodeModifier = (NodesModifierData*)modifier;
	
	if (nodeModifier->settings.properties == nullptr)
		return;

	auto group = nodeModifier->node_group;

	// Loop through modifier data and get the values:
	for (IDProperty* property : blender::list_range((IDProperty*)nodeModifier->settings.properties->data.group.first)) {
		if (strstr(property->name, "_use_attribute") || strstr(property->name, "_attribute_name")) {
			continue;
		}

		if (doesPropertyUseAttribute(property->name, nodeModifier)) {
			continue;
		}

		auto socket = getSocketForProperty(property, group);

		if (socket == nullptr) {
			continue;
		}

		auto propertyInfo = ms::PropertyInfo::create();

		switch (property->type) {
		case IDP_INT: {
			if (socket->type == SOCK_BOOLEAN) {
				propertyInfo->set(IDP_Int(property), 0, 1);
			}
			else {
				auto defaultValue = (bNodeSocketValueInt*)socket->default_value;
				propertyInfo->set(IDP_Int(property), defaultValue->min, defaultValue->max);
			}
			break;
		}
		case IDP_FLOAT: {
			auto defaultValue = (bNodeSocketValueFloat*)socket->default_value;
			propertyInfo->set(IDP_Float(property), defaultValue->min, defaultValue->max);
			break;
		}
		case IDP_DOUBLE: {
			auto defaultValue = (bNodeSocketValueFloat*)socket->default_value;
			propertyInfo->set((float)IDP_Double(property), defaultValue->min, defaultValue->max);
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
		case IDP_STRING: {
			auto val = IDP_String(property);
			propertyInfo->set(val, strlen(val));
			break;
		}
		default:
			continue;
		}

		propertyInfo->path = msblenUtils::get_path(obj);
		propertyInfo->name = socket->name;
		propertyInfo->modifierName = modifier->name;
		propertyInfo->propertyName = property->name;
		propertyInfo->sourceType = ms::PropertyInfo::SourceType::GEO_NODES;
		propertyManager->add(propertyInfo);
	}
}

void addCustomProperties(const Object* obj, ms::TransformPtr dst, ms::PropertyManager* propertyManager, msblenContextPathProvider& paths) {
	dst->propertiesHash = 0;

	if (obj->id.properties == nullptr) {
		return;
	}

	for (auto property : blender::list_range((IDProperty*)obj->id.properties->data.group.first)) {
		if (property->ui_data == nullptr && property->type != IDP_STRING) {
			continue;
		}

		auto propertyInfo = ms::PropertyInfo::create();
		switch (property->type) {
		case IDP_INT: {
			auto uiData = (IDPropertyUIDataInt*)property->ui_data;
			propertyInfo->set(IDP_Int(property), uiData->min, uiData->max);
			break;
		}
		case IDP_FLOAT:
		{
			auto uiData = (IDPropertyUIDataFloat*)property->ui_data;
			propertyInfo->set(IDP_Float(property), uiData->min, uiData->max);
			break;
		}
		case IDP_DOUBLE: {
			auto uiData = (IDPropertyUIDataFloat*)property->ui_data;
			propertyInfo->set((float)IDP_Double(property), uiData->min, uiData->max);
			break;
		}
		case IDP_STRING: {
			auto val = IDP_String(property);
			propertyInfo->set(val, property->len + 1); // strlen(val));
			break;
		}
		case IDP_ARRAY: {
			switch (property->subtype) {
			case IDP_INT: {
				auto uiData = (IDPropertyUIDataInt*)property->ui_data;
				propertyInfo->set((int*)IDP_Array(property), uiData->min, uiData->max, property->len);
				break;
			}
			case IDP_FLOAT: {
				auto uiData = (IDPropertyUIDataFloat*)property->ui_data;
				propertyInfo->set((float*)IDP_Array(property), uiData->min, uiData->max, property->len);
				break;
			}
			case IDP_DOUBLE: {
				auto uiData = (IDPropertyUIDataFloat*)property->ui_data;

				// Convert double array to floats:
				double* doubleArray = (double*)IDP_Array(property);
				int arrayLength = sizeof(double) * property->len;
				float* floatArray = new float[arrayLength];
				for (int i = 0; i < arrayLength; i++)
				{
					floatArray[i] = (float)doubleArray[i];
				}

				propertyInfo->set(floatArray, uiData->min, uiData->max, property->len);
				break;
			}
			}
			break;
		}
		default:
			continue;
		}

		propertyInfo->path = paths.get_path(obj);
		propertyInfo->name = std::string(property->name);
		propertyInfo->modifierName = "";
		propertyInfo->propertyName = std::string(property->name);
		propertyInfo->sourceType = ms::PropertyInfo::SourceType::CUSTOM_PROPERTY;
		propertyManager->add(propertyInfo);

		dst->propertiesHash += propertyInfo->hash();
		dst->propertiesHash += ms::vhash(propertyInfo->data);
	}
}

bool msblenModifiers::doesObjectHaveCustomProperties(const Object* obj) {
	if (!obj->id.properties) {
		return false;
	}

	for (auto property : blender::list_range((IDProperty*)obj->id.properties->data.group.first)) {
		if (property->ui_data == nullptr && property->type != IDP_STRING) {
			continue;
		}

		return true;
	}

	return false;
}

void msblenModifiers::exportProperties(const Object* obj, ms::TransformPtr dst, ms::PropertyManager* propertyManager, msblenContextPathProvider& paths)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	blender::BObject bObj(obj);
	auto modifiers = bObj.modifiers();
	for (auto it = modifiers.begin(); it != modifiers.end(); ++it) {
		auto modifier = *it;

		addModifierProperties(modifier, obj, propertyManager);
	}

	addCustomProperties(obj, dst, propertyManager, paths);
}

void setProperty(const Object* obj, IDProperty* property, ms::PropertyInfo& receivedProp) {
	switch (receivedProp.type) {
	case ms::PropertyInfo::Type::Int: {
		IDP_Int(property) = receivedProp.get<int>();
		break;
	}
	case ms::PropertyInfo::Type::Float: {
		if (property->type == IDP_DOUBLE) {
			IDP_Double(property) = (double)receivedProp.get<float>();
		}
		else {
			IDP_Float(property) = receivedProp.get<float>();
		}
		break;
	}
	case ms::PropertyInfo::Type::String: {
		receivedProp.copy(IDP_String(property));
		property->totallen = property->len = receivedProp.getArrayLength() + 1;
		break;
	}
	case ms::PropertyInfo::Type::FloatArray:
	case ms::PropertyInfo::Type::IntArray:
		if (property->subtype == IDP_DOUBLE) {
			// Convert float array to doubles:
			float* floatArray = receivedProp.getArray<float>();
			int arrayLength = receivedProp.getArrayLength();
			for (int i = 0; i < arrayLength; i++)
			{
				((double*)IDP_Array(property))[i] = (double)floatArray[i];
			}
		}
		else {
			receivedProp.copy(IDP_Array(property));
		}
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

void setProperties(const Object* obj, ms::PropertyInfo& receivedProp, std::string name, ListBase* listBase)
{
	for (auto property : blender::list_range((IDProperty*)listBase->first)) {
		if (property->name == name) {
			setProperty(obj, property, receivedProp);
		}
	}
}

void applyGeoNodeProperty(const Object* obj, ms::PropertyInfo& receivedProp) {
	auto modifier = msblenUtils::FindModifier(obj, receivedProp.modifierName);

	// Should never happen but just in case:
	if (!modifier) {
		return;
	}

	auto nodeModifier = (NodesModifierData*)modifier;

	setProperties(obj, receivedProp, receivedProp.propertyName, &nodeModifier->settings.properties->data.group);
}

void applyCustomProperty(const Object* obj, ms::PropertyInfo& receivedProp) {
	if (obj->id.properties) {
		setProperties(obj, receivedProp, receivedProp.name, &obj->id.properties->data.group);
	}
}


void msblenModifiers::importProperties(std::vector<ms::PropertyInfo> props) {
	if (props.size() == 0) {
		return;
	}

	debug_log("importProperties");

	std::unique_lock<std::mutex> lock(m_mutex);
	// Apply returned properties:
	for (auto& receivedProp : props) {
		if (receivedProp.type == ms::PropertyInfo::Type::Int)
			debug_log(mu::Format("importing: %s: %d %d", receivedProp.name.c_str(), receivedProp.get<int>(), receivedProp.sourceType));

		auto obj = msblenUtils::get_object_from_path(receivedProp.path);

		// Should never happen but just in case:
		if (!obj) {
			debug_log(mu::Format("cannot find object: %s", receivedProp.path.c_str()));
			continue;
		}

		switch (receivedProp.sourceType)
		{
		case ms::PropertyInfo::SourceType::GEO_NODES:
			applyGeoNodeProperty(obj, receivedProp);
			break;
		case ms::PropertyInfo::SourceType::CUSTOM_PROPERTY:
			applyCustomProperty(obj, receivedProp);
			break;
		}

		blender::BlenderPyID bID(obj);
		bID.update_tag();
	}
}

#endif // BLENDER_VERSION < 300

} // namespace blender 
