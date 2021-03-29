#pragma once

#include <BKE_context.h>
#include "BlenderPyCommon.h"

#include "msblenMacros.h"

#if BLENDER_VERSION < 280
#else
struct Depsgraph;
#endif

namespace blender
{

class BlenderPyContext
{
public:
    MSBLEN_BOILERPLATE2(BlenderPyContext, bContext)

    static BlenderPyContext get();
    Main* data();
    Scene* scene();
    Depsgraph* evaluated_depsgraph_get();
};

} // namespace blender
