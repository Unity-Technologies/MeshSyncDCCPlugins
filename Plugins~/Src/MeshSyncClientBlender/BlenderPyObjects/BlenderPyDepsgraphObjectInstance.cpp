#include "pch.h"
#include "BlenderPyCommon.h"
#include "BlenderPyObjects/BlenderPyDepsgraphObjectInstance.h"

namespace blender {

    PropertyRNA* BlenderPyDepsgraphObjectInstance_instance_object = nullptr;
    PropertyRNA* BlenderPyDepsgraphObjectInstance_is_instance = nullptr;
    PropertyRNA* BlenderPyDepsgraphObjectInstance_world_matrix = nullptr;
    PropertyRNA* BlenderPyDepsgraphObjectInstance_parent = nullptr;
    PropertyRNA* BlenderPyDepsgraphObjectInstance_object = nullptr;

    Object* BlenderPyDepsgraphInstance::instance_object() {
        auto objectInstanceProp = (PointerPropertyRNA*)BlenderPyDepsgraphObjectInstance_instance_object;
        auto object = objectInstanceProp->get(&m_instance);

        if (object.type == nullptr || object.data == nullptr) {
            return nullptr;
        }

        return (Object*)object.data;
    }

    bool BlenderPyDepsgraphInstance::is_instance() {
        auto booleanProp = (BoolPropertyRNA*)BlenderPyDepsgraphObjectInstance_is_instance;
        return booleanProp->get(&m_instance);
    }

    void BlenderPyDepsgraphInstance::world_matrix(mu::float4x4* result)
    {
        auto floatProp = (FloatPropertyRNA*)BlenderPyDepsgraphObjectInstance_world_matrix;
        floatProp->getarray(&m_instance, &(result->m[0][0]));
    }

    Object* BlenderPyDepsgraphInstance::parent() {
        auto pointerProp = (PointerPropertyRNA*)BlenderPyDepsgraphObjectInstance_parent;
        auto parent = pointerProp->get(&m_instance);

        return (Object*)parent.data;
    }

    Object* BlenderPyDepsgraphInstance::object() {
        auto objectInstanceProp = (PointerPropertyRNA*)BlenderPyDepsgraphObjectInstance_object;
        auto object = objectInstanceProp->get(&m_instance);

        if (object.type == nullptr || object.data == nullptr) {
            return nullptr;
        }

        return (Object*)object.data;
    }

} // namespace blender
