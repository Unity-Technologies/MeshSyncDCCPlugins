#pragma once
#include "pch.h"

namespace blender {

class BlenderPyImage {
public:
	static int GetPixelsArrayCount(Image* image);
	static float* GetPixels(Image* image);
};


} // namespace blender
