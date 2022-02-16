#pragma once

#include <BKE_context.h> //bContext
#include "MeshUtils/muMath.h"
#include <DNA_modifier_types.h>
#include <RNA_define.h> 

namespace blender
{
    class BlenderPyDepgraphUpdate
    {
    public:

        void updates_begin(CollectionPropertyIterator* it, Depsgraph* graph);
        void updates_next(CollectionPropertyIterator* it);
        void updates_end(CollectionPropertyIterator* it);
        PointerRNA updates_get(CollectionPropertyIterator* it);

        bool id_type_updated(int type, Depsgraph* graph);
    };

} // namespace blender
