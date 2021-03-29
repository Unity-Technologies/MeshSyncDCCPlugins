#include "pch.h"
#include "BlenderPyCommon.h"
#include "BlenderPyContext.h"


namespace blender
{
extern bContext *g_context;

StructRNA* BlenderPyContext::s_type;
static PropertyRNA* BContext_blend_data;
static PropertyRNA* BContext_scene;
static FunctionRNA* BContext_evaluated_depsgraph_get;
static FunctionRNA* BDepsgraph_update;



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
