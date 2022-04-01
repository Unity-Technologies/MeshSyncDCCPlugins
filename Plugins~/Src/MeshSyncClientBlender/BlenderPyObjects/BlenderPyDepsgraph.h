#pragma once
#include <BKE_context.h> //bContext
#include "MeshUtils/muMath.h"

namespace blender
{

    class BlenderPyDepsgraph
    {
    public:
        BlenderPyDepsgraph(Depsgraph* depsgraph);
        void object_instances_begin(CollectionPropertyIterator* it);
        void object_instances_end(CollectionPropertyIterator* it);
        void object_instances_next(CollectionPropertyIterator* it);
        PointerRNA object_instances_get(CollectionPropertyIterator* it);

    private:
        Depsgraph* m_depsgraph;
    };

}