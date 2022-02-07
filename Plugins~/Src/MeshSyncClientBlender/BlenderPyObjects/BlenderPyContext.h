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
        Depsgraph* evaluated_depsgraph_get();

        static void UpdateDepsgraph(Depsgraph* depsgraph);

        void object_instances_begin(CollectionPropertyIterator* it, Depsgraph* depsgrah);
        void object_instances_end(CollectionPropertyIterator* it);
        void object_instances_next(CollectionPropertyIterator* it);
        PointerRNA object_instances_get(CollectionPropertyIterator* it);


        Object* instance_object_get(PointerRNA instance);
        bool object_instances_is_instance(PointerRNA instance);
        void world_matrix_get(PointerRNA* instance, mu::float4x4* world_matrix);
        Object* instance_parent_get(PointerRNA* instance);
    };

} // namespace blender
