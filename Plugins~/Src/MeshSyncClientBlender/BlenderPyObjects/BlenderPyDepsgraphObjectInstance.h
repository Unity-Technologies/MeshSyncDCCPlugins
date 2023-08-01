#pragma once

#include <BKE_context.h> //bContext
#include "MeshUtils/muMath.h"

namespace blender
{
    class BlenderPyDepsgraphInstance
    {
        public:
            BlenderPyDepsgraphInstance(PointerRNA& instance) : m_instance(instance) {}

            Object* instance_object();
            bool is_instance();
            void world_matrix(mu::float4x4* world_matrix);
            Object* parent();
            Object* object();
    public:
        PointerRNA& m_instance;
    };

} // namespace blender
