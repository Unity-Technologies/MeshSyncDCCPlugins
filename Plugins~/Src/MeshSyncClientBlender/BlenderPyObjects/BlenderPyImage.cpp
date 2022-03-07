#include "BlenderPyImage.h"

namespace blender {

PropertyRNA* BlenderPyImage_pixels = nullptr;
PropertyRNA* BlenderPyImage_size = nullptr;
PropertyRNA* BlenderPyImage_channels = nullptr;

std::vector<float> BlenderPyImage::GetPixels(Image* image) {
    auto pixels_pp = (FloatPropertyRNA*)BlenderPyImage_pixels;

    PointerRNA rna;
    rna.data = image;
    rna.owner_id = (ID*)image; //Some functions use .owner_id to get the data, some use .data - I don't know why

    int lenArray[3]{ 0, 0, 0 };
    auto len = BlenderPyImage_pixels->getlength(&rna, lenArray);

    std::vector<float> pixArray(len);
    pixels_pp->getarray(&rna, pixArray.data());
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
