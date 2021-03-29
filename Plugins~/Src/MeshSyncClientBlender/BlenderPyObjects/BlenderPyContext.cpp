#include "pch.h"
#include "BlenderPyContext.h"
#include "BlenderPyCommon.h"

namespace blender {

extern bContext *g_context;

StructRNA* BlenderPyContext::s_type;
PropertyRNA* BContext_blend_data = nullptr;
PropertyRNA* BContext_scene = nullptr;
FunctionRNA* BContext_evaluated_depsgraph_get = nullptr;


BlenderPyContext BlenderPyContext::get()
{
    return BlenderPyContext(g_context);
}
Main* BlenderPyContext::data()
{
    return (Main*)get_pointer(m_ptr, BContext_blend_data);
}
Scene* BlenderPyContext::scene()
{
    return (Scene*)get_pointer(m_ptr, BContext_scene);
}

Depsgraph* BlenderPyContext::evaluated_depsgraph_get()
{
    return call<bContext, Depsgraph*>(m_ptr, BContext_evaluated_depsgraph_get);
}

} // namespace blender
