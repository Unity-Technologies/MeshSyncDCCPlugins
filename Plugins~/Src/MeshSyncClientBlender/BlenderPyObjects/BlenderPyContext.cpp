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
    return call<bContext, Depsgraph*>(m_ptr, BlenderPyContext_evaluated_depsgraph_get);
}


void BlenderPyContext::UpdateDepsgraph(Depsgraph* depsgraph) {

    struct DepsGraphInChar {
        char Buffer[1240];
    };

    DepsGraphInChar* charGraph = reinterpret_cast<DepsGraphInChar*>(depsgraph);

    //[Note-sin: 2021-5-13] Manually update id_type_updated[INDEX_ID_AC] so that depsgraph update will invoke the callbacks
    const size_t ID_TYPE_UPDATED_OFFSET = 273;
    charGraph->Buffer[ID_TYPE_UPDATED_OFFSET + INDEX_ID_AC] = 1;

    call<Depsgraph, void>(depsgraph, BlenderPyContext_depsgraph_update);
}


} // namespace blender
