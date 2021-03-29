#pragma once

#include <BKE_context.h>

#include "msblenMacros.h"

#if BLENDER_VERSION < 280
#else
struct Depsgraph;
#endif

namespace blender
{

    class PyContext
    {
    public:
        MSBLEN_BOILERPLATE2(PyContext, bContext)

        static PyContext get();
        Main* data();
        Scene* scene();
        Depsgraph* evaluated_depsgraph_get();
        void EvaluateDepsgraph();
    };



} // namespace blender
