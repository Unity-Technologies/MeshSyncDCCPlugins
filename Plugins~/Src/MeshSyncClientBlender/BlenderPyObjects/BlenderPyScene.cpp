#include "pch.h"
#include "BlenderPyScene.h"
#include "BlenderPyCommon.h" //call

namespace blender
{
extern bContext *g_context;

StructRNA* BlenderPyScene::s_type;
PropertyRNA* BlenderPyScene_frame_start = nullptr;
PropertyRNA* BlenderPyScene_frame_end = nullptr;
PropertyRNA* BlenderPyScene_frame_current = nullptr;
FunctionRNA* BlenderPyScene_frame_set = nullptr;

//----------------------------------------------------------------------------------------------------------------------

int BlenderPyScene::fps() const  { return m_ptr->r.frs_sec; }
int BlenderPyScene::frame_start() const  { return GetInt(m_ptr, BlenderPyScene_frame_start); }
int BlenderPyScene::frame_end() const { return GetInt(m_ptr, BlenderPyScene_frame_end); }
int BlenderPyScene::GetCurrentFrame() const{ return GetInt(m_ptr, BlenderPyScene_frame_current); }

void BlenderPyScene::SetCurrentFrame(int frame, Depsgraph* depsgraph) {
    SetInt(m_ptr, BlenderPyScene_frame_current, frame);

    struct DepsGraphInChar {
        char Buffer[1240]; //the size is actually different per Blender ver, per OS. What's important is the offset.
    };

#if BLENDER_VERSION <= 283
    const size_t ID_TYPE_UPDATED_OFFSET = 41;
#else
    const size_t ID_TYPE_UPDATED_OFFSET = 265;
#endif

    //[Note-sin: 2021-5-13] Since we are modifying frame_current directly, and not using frame_set(),
    //we have to manually update id_type_updated so that depsgraph.update will invoke the handlers: depsgraph_update_pre, etc
    DepsGraphInChar* charGraph = reinterpret_cast<DepsGraphInChar*>(depsgraph);

    //Offset is different between Blender Debug and Release, and maybe different among different Blender versions !
    charGraph->Buffer[ID_TYPE_UPDATED_OFFSET + INDEX_ID_SCE] = 1;

}

void BlenderPyScene::frame_set(int f, float subf)
{
    call<Scene, void, int, float>(m_ptr, BlenderPyScene_frame_set, f, subf);
}



} // namespace blender
