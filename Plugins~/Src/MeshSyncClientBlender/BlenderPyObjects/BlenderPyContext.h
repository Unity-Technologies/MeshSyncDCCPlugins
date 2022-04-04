#pragma once

#include <BKE_context.h> //bContext
#include "msblenMacros.h" //MSBLEN_BOILERPLATE2
#include "MeshUtils/muMath.h"

namespace blender
{

    class BlenderPyContext
    {
    public:
        MSBLEN_BOILERPLATE2(BlenderPyContext, bContext)

        static BlenderPyContext get();
        Main* data();
        Scene* scene();
        ViewLayer* viewLayer();
        Depsgraph* evaluated_depsgraph_get();

        static void UpdateDepsgraph(Depsgraph* depsgraph);
    };

} // namespace blender
