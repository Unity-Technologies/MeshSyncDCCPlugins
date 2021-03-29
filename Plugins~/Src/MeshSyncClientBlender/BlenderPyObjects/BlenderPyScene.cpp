#include "pch.h"
#include "BlenderPyScene.h"
#include "BlenderPyCommon.h" //call

namespace blender
{
extern bContext *g_context;

StructRNA* BlenderPyScene::s_type;
PropertyRNA* BScene_frame_start;
PropertyRNA* BScene_frame_end;
PropertyRNA* BScene_frame_current;
FunctionRNA* BScene_frame_set;

//----------------------------------------------------------------------------------------------------------------------

int BlenderPyScene::fps() { return m_ptr->r.frs_sec; }
int BlenderPyScene::frame_start() { return get_int(m_ptr, BScene_frame_start); }
int BlenderPyScene::frame_end() { return get_int(m_ptr, BScene_frame_end); }
int BlenderPyScene::frame_current() { return get_int(m_ptr, BScene_frame_current); }

void BlenderPyScene::frame_set(int f, float subf)
{
    call<Scene, void, int, float>(m_ptr, BScene_frame_set, f, subf);
}



} // namespace blender
