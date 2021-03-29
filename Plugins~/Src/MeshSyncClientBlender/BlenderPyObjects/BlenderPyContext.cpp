#include "pch.h"
#include "BlenderPyCommon.h"
#include "BlenderPyContext.h"


namespace blender
{
extern bContext *g_context;
StructRNA* PyContext::s_type;
static PropertyRNA* BContext_blend_data;
static PropertyRNA* BContext_scene;
static FunctionRNA* BContext_evaluated_depsgraph_get;
static FunctionRNA* BDepsgraph_update;



PyContext PyContext::get()
{
    return PyContext(g_context);
}
Main* PyContext::data()
{
    return (Main*)get_pointer(m_ptr, BContext_blend_data);
}
Scene* PyContext::scene()
{
    return (Scene*)get_pointer(m_ptr, BContext_scene);
}

Depsgraph* PyContext::evaluated_depsgraph_get()
{
    return call<bContext, Depsgraph*>(m_ptr, BContext_evaluated_depsgraph_get);
}

void PyContext::EvaluateDepsgraph() {
    Depsgraph* depsgraph = evaluated_depsgraph_get();
    call<Depsgraph, void>(depsgraph, BDepsgraph_update);
}


} // namespace blender
