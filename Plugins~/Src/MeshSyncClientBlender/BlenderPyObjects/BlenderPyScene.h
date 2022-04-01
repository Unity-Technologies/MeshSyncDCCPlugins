#pragma once

#include "DNA_scene_types.h" //Scene
#include "msblenMacros.h" //MSBLEN_BOILERPLATE2

#include "BlenderPyID.h" //BID
#include "../msblenBinder.h" //list_range

#include "msblenUtils.h" // get_name

namespace blender {

class BlenderPyScene {
public:
    MSBLEN_BOILERPLATE2(BlenderPyScene, Scene)
    MSBLEN_COMPATIBLE(BlenderPyID)

    int fps() const ;
    int frame_start() const ;
    int frame_end() const ;
    int GetCurrentFrame() const;
    void SetCurrentFrame(int frame, Depsgraph* depsgraph);

    void frame_set(int f, float subf = 0.0f);

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

    template<class Body>
    void each_selection(const Body& body)
    {
        each_objects([&](Object *obj) {
            BObject bo(obj);
            if (bo.is_selected())
                body(obj);
        });
    }

    Object* get_object_by_name(std::string objName) {
        for (auto* c : list_range((CollectionChild*)m_ptr->master_collection->children.first)) {
            for (auto* o : list_range((CollectionObject*)c->collection->gobject.first)) {
                auto obj = o->ob;
                auto name = msblenUtils::get_name(obj);

                if (name == objName) {
                    return obj;
                }
            }
        }

        return nullptr;
    }
};


} // namespace blender
