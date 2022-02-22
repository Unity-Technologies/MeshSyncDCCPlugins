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
PropertyRNA* BlenderPyContext_image_pixels = nullptr;
PropertyRNA* BlenderPyContext_image_file_format = nullptr;
PropertyRNA* BlenderPyContext_image_size = nullptr;
PropertyRNA* BlenderPyContext_image_channels = nullptr;


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


void BlenderPyContext::UpdateDepsgraph(Depsgraph* depsgraph) {
    call<Depsgraph, void>(g_context, depsgraph, BlenderPyContext_depsgraph_update);
}

float* BlenderPyContext::GetPixels(Image* image) {
    auto pp = (FloatPropertyRNA*)BlenderPyContext_image_pixels;

    auto epp = (EnumPropertyRNA*)BlenderPyContext_image_file_format;
    
    auto sizepp = (IntPropertyRNA*)BlenderPyContext_image_size;

    auto channelspp = (IntPropertyRNA*)BlenderPyContext_image_channels;
    

    PointerRNA rna;
    rna.data = image;
    rna.owner_id = (ID *)image;

    auto format = epp->get(&rna);

    int sizearray[2] { 0, 0 };
    sizepp->getarray(&rna, (int*)sizearray);

    int x = sizearray[0];
    int y = sizearray[1];

    auto channels = channelspp->get(&rna);
    auto total = x * y * channels;

    int lenArray[3]{ 0, 0, 0 };
    auto len = BlenderPyContext_image_pixels->getlength(&rna, lenArray);

    float* pixArray = new float[len];
    pp->getarray(&rna, pixArray);
    return pixArray;
}

int BlenderPyContext::GetPixelsLength(Image* image) {
    return get_length(image, BlenderPyContext_image_pixels);
}

} // namespace blender
