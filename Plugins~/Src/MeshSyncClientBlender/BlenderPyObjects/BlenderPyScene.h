#pragma once

#include "DNA_scene_types.h" //Scene
#include "msblenMacros.h" //MSBLEN_BOILERPLATE2

#include "BlenderPyID.h" //BID

namespace blender {

class BlenderPyScene {
public:
    MSBLEN_BOILERPLATE2(BlenderPyScene, Scene)
    MSBLEN_COMPATIBLE(BlenderPyID)

    int fps();
    int frame_start();
    int frame_end();
    int GetCurrentFrame() const;
    void SetCurrentFrame(int frame);

    void frame_set(int f, float subf = 0.0f);

#if BLENDER_VERSION < 280
    template<class Body>
    void each_objects(const Body& body)
    {
        for (auto *base : list_range((Base*)m_ptr->base.first)) {
            body(base->object);
        }
    }
#else
    template<class Body>
    void each_objects_impl(const Body& body, CollectionChild *cc)
    {
        for (auto *c : list_range(cc)) {
            each_objects_impl(body, (CollectionChild*)c->collection->children.first);
            for (auto *o : list_range((CollectionObject*)c->collection->gobject.first))
                body(o->ob);
        }
    }

    template<class Body>
    void each_objects(const Body& body)
    {
        each_objects_impl(body, (CollectionChild*)m_ptr->master_collection->children.first);
        for (auto *o : list_range((CollectionObject*)m_ptr->master_collection->gobject.first))
            body(o->ob);
    }
#endif

    template<class Body>
    void each_selection(const Body& body)
    {
        each_objects([&](Object *obj) {
            BObject bo(obj);
            if (bo.is_selected())
                body(obj);
        });
    }
};


} // namespace blender
