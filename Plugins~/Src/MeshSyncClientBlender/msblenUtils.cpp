#include "pch.h"

#include "msblenBinder.h"
#include "msblenUtils.h"

#include "BlenderPyObjects/BlenderPyScene.h"

namespace bl = blender;
namespace msblenUtils {
    std::string get_name(const Material* obj)
    {
        std::string ret;
        if (obj) {
            ret.assign(obj->id.name + 2);
            mu::SanitizeNodeName(ret);
        }
        return ret;
    }

    std::string get_name(const Object* obj)
    {
        std::string ret;
        if (obj) {
            ret.assign(obj->id.name + 2);
            mu::SanitizeNodeName(ret);
        }
        return ret;
    }

    std::string get_name(const Bone* obj)
    {
        std::string ret;
        if (obj) {
            ret.assign(obj->name);
            mu::SanitizeNodeName(ret);
        }
        return ret;
    }

    std::string get_path(const Object* obj)
    {
        std::string ret;
        if (obj->parent) {
            if (obj->partype == PARBONE) {
                if (auto bone = find_bone(obj->parent, obj->parsubstr)) {
                    ret += get_path(obj->parent, bone);
                }
            }
            else {
                ret += get_path(obj->parent);
            }
        }
        ret += '/';
        ret += get_name(obj);
        return ret;
    }
    std::string get_path(const Object* arm, const Bone* obj)
    {
        std::string ret;
        if (obj->parent)
            ret += get_path(arm, obj->parent);
        else
            ret += get_path(arm);
        ret += '/';
        ret += get_name(obj);
        return ret;
    }

    Object* get_object_from_path(std::string path) {
        auto lastIndexOfDivider = path.find_last_of('/');
        if (lastIndexOfDivider < 0) {
            return nullptr;
        }

        auto objName = path.substr(lastIndexOfDivider + 1);

        bl::BlenderPyScene scene = bl::BlenderPyScene(bl::BlenderPyContext::get().scene());
        return scene.get_object_by_name(objName);
    }

    bool visible_in_render(const Object* obj)
    {
        return !bl::BObject(obj).hide_render();
    }
    bool visible_in_viewport(const Object* obj)
    {
        return !bl::BObject(obj).hide_viewport();
    }

    bool visible_in_collection(LayerCollection* layerCollection, const Object* obj) {
        // Check if the object is in the layer collection, if it is, check if the layer is excluded:
        for (auto collectionObject : blender::list_range((CollectionObject*)layerCollection->collection->gobject.first)) {
            if (collectionObject->ob == obj) {
                if (!(layerCollection->flag & LAYER_COLLECTION_EXCLUDE)) {
                    return true;
                }
            }
        }

        // Check child layer collections:
        for (auto childCollection : blender::list_range((LayerCollection*)layerCollection->layer_collections.first)) {
            if (visible_in_collection(childCollection, obj)) {
                return true;
            }
        }

        return false;
    }

    bool visible_in_collection(const Object* obj) {
        auto viewLayer = bl::BlenderPyContext::get().viewLayer();

        for (auto layerCollection : blender::list_range((LayerCollection*)viewLayer->layer_collections.first)) {
            if (visible_in_collection(layerCollection, obj)) {
                return true;
            }
        }

        return false;
    }

    const ModifierData* FindModifier(const Object* obj, ModifierType type)
    {
        for (auto* it = (ModifierData*)obj->modifiers.first; it != nullptr; it = it->next)
            if (it->type == type)
                return it;
        return nullptr;;
    }

    const ModifierData* FindModifier(const Object* obj, const std::string name)
    {
        for (auto* it = (ModifierData*)obj->modifiers.first; it != nullptr; it = it->next)
            if (it->name == name)
                return it;
        return nullptr;
    }

    Bone* find_bone_recursive(Bone* bone, const char* name)
    {
        if (strcmp(bone->name, name) == 0) {
            return bone;
        }
        else {
            for (auto* child = (Bone*)bone->childbase.first; child != nullptr; child = child->next) {
                auto* found = find_bone_recursive(child, name);
                if (found)
                    return found;
            }
        }
        return nullptr;
    }

    Bone* find_bone(Object* obj, const char* name)
    {
        if (!obj) { return nullptr; }
        auto* arm = (bArmature*)obj->data;
        for (auto* bone = (Bone*)arm->bonebase.first; bone != nullptr; bone = bone->next)
        {
            auto found = find_bone_recursive(bone, name);
            if (found)
                return found;
        }
        return nullptr;
    }

    bPoseChannel* find_pose(Object* obj, const char* name)
    {
        if (!obj || !obj->pose) { return nullptr; }
        for (auto* it = (bPoseChannel*)obj->pose->chanbase.first; it != nullptr; it = it->next)
            if (std::strcmp(it->name, name) == 0)
                return it;
        return nullptr;
    }

    bool is_mesh(const Object* obj) { return obj->type == OB_MESH; }
    bool is_camera(const Object* obj) { return obj->type == OB_CAMERA; }
    bool is_light(const Object* obj) { return obj->type == OB_LAMP; }
    bool is_armature(const Object* obj) { return obj->type == OB_ARMATURE; }

    bool is_valid_pointer(void* ptr) { return ptr != nullptr && ptr != ((void*)-1); }
}
