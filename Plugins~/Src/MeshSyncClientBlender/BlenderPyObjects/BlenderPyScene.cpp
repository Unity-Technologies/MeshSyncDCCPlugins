#include "pch.h"
#include "BlenderPyScene.h"
#include "BlenderPyCommon.h" //call

namespace blender
{
extern bContext *g_context;

StructRNA* BlenderPyScene::s_type;
PropertyRNA* BlenderPyScene_frame_start;
PropertyRNA* BlenderPyScene_frame_end;
PropertyRNA* BlenderPyScene_frame_current;
FunctionRNA* BlenderPyScene_frame_set;

//----------------------------------------------------------------------------------------------------------------------

int BlenderPyScene::fps() const  { return m_ptr->r.frs_sec; }
int BlenderPyScene::frame_start() const  { return GetInt(m_ptr, BlenderPyScene_frame_start); }
int BlenderPyScene::frame_end() const { return GetInt(m_ptr, BlenderPyScene_frame_end); }
int BlenderPyScene::GetCurrentFrame() const{ return GetInt(m_ptr, BlenderPyScene_frame_current); }

void BlenderPyScene::SetCurrentFrame(int frame) {
    SetInt(m_ptr, BlenderPyScene_frame_current, frame);    
}

void BlenderPyScene::frame_set(int f, float subf)
{
    call<Scene, void, int, float>(m_ptr, BlenderPyScene_frame_set, f, subf);
}



} // namespace blender
