#pragma once

#include <BKE_context.h> //bContext
#include "MeshUtils/muMath.h"
#include <DNA_modifier_types.h>
#include <RNA_define.h> 

namespace blender
{
    class BlenderPyNodeTree
    {
    public:
        void inputs_begin(CollectionPropertyIterator* it, bNodeTree* nodeTree);
        void inputs_next(CollectionPropertyIterator* it);
        void inputs_end(CollectionPropertyIterator* it);
        PointerRNA inputs_get(CollectionPropertyIterator* it);
    };

} // namespace blender
