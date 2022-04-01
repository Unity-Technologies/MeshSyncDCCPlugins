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

Depsgraph* BlenderPyContext::evaluated_depsgraph_get()
{
    return call<bContext, Depsgraph*>(g_context, m_ptr, BlenderPyContext_evaluated_depsgraph_get);
}

ViewLayer* BlenderPyContext::viewLayer()
{
    return (ViewLayer*)get_pointer(m_ptr, BlenderPyContext_viewlayer);
}


void BlenderPyContext::UpdateDepsgraph(Depsgraph* depsgraph) {
    call<Depsgraph, void>(g_context, depsgraph, BlenderPyContext_depsgraph_update);
}

} // namespace blender
