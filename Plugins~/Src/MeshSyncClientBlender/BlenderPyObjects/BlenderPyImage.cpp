#include "BlenderPyImage.h"

namespace blender {

PropertyRNA* BlenderPyImage_pixels = nullptr;
PropertyRNA* BlenderPyImage_file_format = nullptr;
PropertyRNA* BlenderPyImage_size = nullptr;
PropertyRNA* BlenderPyImage_channels = nullptr;

float* BlenderPyImage::GetPixels(Image* image) {
    auto pixels_pp = (FloatPropertyRNA*)BlenderPyImage_pixels;

    auto size_pp = (IntPropertyRNA*)BlenderPyImage_size;

    auto channels_pp = (IntPropertyRNA*)BlenderPyImage_channels;


    PointerRNA rna;
    rna.data = image;
    rna.owner_id = (ID*)image; //Some functions use .owner_id to get the data, some use .data - I don't know why

    /*
    int sizearray[2]{ 0, 0 };
    size_pp->getarray(&rna, (int*)sizearray);

    int width = sizearray[0];
    int height = sizearray[1];
    auto channels = channels_pp->get(&rna);

    auto total = width * height * channels;*/

    int lenArray[3]{ 0, 0, 0 };
    auto len = BlenderPyImage_pixels->getlength(&rna, lenArray);

    float* pixArray = new float[len];
    pixels_pp->getarray(&rna, pixArray);
    return pixArray;
}

int BlenderPyImage::GetPixelsArrayCount(Image* image) {
    PointerRNA rna;
    rna.data = image;
    rna.owner_id = (ID*)image;

    int lenArray[3]{ 0, 0, 0 };
    auto len = BlenderPyImage_pixels->getlength(&rna, lenArray);

    return len;
}

} // namespace blender
