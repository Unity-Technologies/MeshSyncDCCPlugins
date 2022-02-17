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

        /*Inputs*/
        void inputs_begin(CollectionPropertyIterator* it, bNodeTree* nodeTree);
        void inputs_next(CollectionPropertyIterator* it);
        void inputs_end(CollectionPropertyIterator* it);
        PointerRNA inputs_get(CollectionPropertyIterator* it);

        /*Outputs*/
        void outputs_begin(CollectionPropertyIterator* it, bNodeTree* nodeTree);
        void outputs_next(CollectionPropertyIterator* it);
        void outputs_end(CollectionPropertyIterator* it);
        PointerRNA outputs_get(CollectionPropertyIterator* it);

        /*Nodes*/
        void nodes_begin(CollectionPropertyIterator* it, const bNodeTree* nodeTree);
        void nodes_next(CollectionPropertyIterator* it);
        void nodes_end(CollectionPropertyIterator* it);
        PointerRNA nodes_get(CollectionPropertyIterator* it);
    };

} // namespace blender
