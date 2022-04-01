#include "pch.h"
#include "BlenderPyContext.h"
#include "BlenderPyCommon.h"

namespace blender {

extern bContext *g_context;

StructRNA* BlenderPyContext::s_type;
PropertyRNA* BlenderPyContext_blend_data = nullptr;
PropertyRNA* BlenderPyContext_scene = nullptr;
FunctionRNA* BlenderPyContext_evaluated_depsgraph_get = nullptr;
FunctionRNA* BlenderPyContext_depsgraph_update = nullptr;
PropertyRNA* BlenderPyContext_depsgraph_object_instances = nullptr;
PropertyRNA* BlenderPyContext_depsgraph_instance_object = nullptr;
PropertyRNA* BlenderPyContext_depsgraph_is_instance = nullptr;
PropertyRNA* BlenderPyContext_depsgraph_world_matrix = nullptr;
PropertyRNA* BlenderPyContext_depsgraph_parent = nullptr;
PropertyRNA* BlenderPyContext_depsgraph_object = nullptr;
PropertyRNA* BlenderPyContext_viewlayer = nullptr;


BlenderPyContext BlenderPyContext::get()
{
    return BlenderPyContext(g_context);
}
Main* BlenderPyContext::data()
{
    return (Main*)get_pointer(m_ptr, BlenderPyContext_blend_data);
}
Scene* BlenderPyContext::scene()
{
    return (Scene*)get_pointer(m_ptr, BlenderPyContext_scene);
}

ViewLayer* BlenderPyContext::viewLayer() {
    return (ViewLayer*)get_pointer(m_ptr, BlenderPyContext_viewlayer);
}

Depsgraph* BlenderPyContext::evaluated_depsgraph_get()
{
    return call<bContext, Depsgraph*>(g_context, m_ptr, BlenderPyContext_evaluated_depsgraph_get);
}


void BlenderPyContext::UpdateDepsgraph(Depsgraph* depsgraph) {
    call<Depsgraph, void>(g_context, depsgraph, BlenderPyContext_depsgraph_update);
}

void BlenderPyContext::object_instances_begin(CollectionPropertyIterator* it, Depsgraph* depsgraph) {

    PointerRNA rna;
    rna.data = depsgraph;

    CollectionPropertyRNA* cprop = (CollectionPropertyRNA*)BlenderPyContext_depsgraph_object_instances;
    
    cprop->begin(it, &rna);
}

void BlenderPyContext::object_instances_end(CollectionPropertyIterator* it) {

    CollectionPropertyRNA* cprop = (CollectionPropertyRNA*)BlenderPyContext_depsgraph_object_instances;
    cprop->end(it);
}

void BlenderPyContext::object_instances_next(CollectionPropertyIterator * it){
    CollectionPropertyRNA* cprop = (CollectionPropertyRNA*)BlenderPyContext_depsgraph_object_instances;
    cprop->next(it);
}

PointerRNA BlenderPyContext::object_instances_get(CollectionPropertyIterator* it) {

    auto collectionProp = (CollectionPropertyRNA*)BlenderPyContext_depsgraph_object_instances;
    return collectionProp->get(it);

}

Object* BlenderPyContext::instance_object_get(PointerRNA instance) {
    auto objectInstanceProp = (PointerPropertyRNA*)BlenderPyContext_depsgraph_instance_object;
    auto object = objectInstanceProp->get(&instance);

    if (object.type == nullptr || object.data == nullptr) {
        return nullptr;
    }

    return (Object*)object.data;
}



bool BlenderPyContext::object_instances_is_instance(PointerRNA object) {
    auto booleanProp = (BoolPropertyRNA*)BlenderPyContext_depsgraph_is_instance;
    return booleanProp->get(&object);
}

void BlenderPyContext::world_matrix_get(PointerRNA* instance, mu::float4x4* result)
{
    auto floatProp = (FloatPropertyRNA*)BlenderPyContext_depsgraph_world_matrix;
    floatProp->getarray(instance, &(result->m[0][0]));
}

Object* BlenderPyContext::instance_parent_get(PointerRNA* instance) {
    auto pointerProp = (PointerPropertyRNA*)BlenderPyContext_depsgraph_parent;
    auto parent = pointerProp->get(instance);

    return (Object*)parent.data;
}

Object* BlenderPyContext::object_get(PointerRNA instance) {
    auto objectInstanceProp = (PointerPropertyRNA*)BlenderPyContext_depsgraph_object;
    auto object = objectInstanceProp->get(&instance);

    if (object.type == nullptr || object.data == nullptr) {
        return nullptr;
    }

    return (Object*)object.data;
}

} // namespace blender
