#pragma once

#include <BKE_context.h> //bContext
#include "msblenMacros.h" //MSBLEN_BOILERPLATE2

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
        ViewLayer* view_layer();

        static void UpdateDepsgraph(Depsgraph* depsgraph);

    };

} // namespace blender
