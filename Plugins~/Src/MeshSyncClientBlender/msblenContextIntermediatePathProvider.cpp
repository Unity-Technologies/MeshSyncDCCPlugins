#include "msblenContextIntermediatePathProvider.h"
#include "msblenUtils.h"
#include <BLI_listbase.h>

std::string msblenContextIntermediatePathProvider::append_id(std::string path, const Object* obj) {
    // If there is no data, no need to append anything:
    if (!obj->data) {
        return path;
    }

    auto data = (ID*)obj->data;

    path += "_" + std::string(data->name + 2);

    // If we already have an object with this name but a different session_uuid, append the session_uuid as well
    auto it = mappedNames.find(data->name);

    if (it == mappedNames.end()) {
        mappedNames.insert(std::make_pair(data->name, data->session_uuid));
    }
    else if (it->second != data->session_uuid)
    {
        path += "_" + std::to_string(data->session_uuid);
    }

    return path;
}

std::string msblenContextIntermediatePathProvider::get_path(const Object* obj, const Bone* bone)
{
    std::string path;
    if (bone) {
        //path = msblenUtils::get_path(obj, bone);
        return get_path_with_suffix(obj, bone);
    }
    else {
        path = "/" + msblenUtils::get_name(obj);
    }

    return append_id(path, obj);
}


std::string msblenContextIntermediatePathProvider::get_path_with_suffix(const Object* obj) {
    std::string ret;
    if (obj->parent) {
        // Build bone path for armatures only, not other objects that are children of armatures:
        if (obj->type == OB_ARMATURE && obj->partype == PARBONE) {
            if (auto bone = msblenUtils::find_bone(obj->parent, obj->parsubstr)) {
                ret += get_path_with_suffix(obj->parent, bone);
            }
        }
        else {
            ret += get_path_with_suffix(obj->parent);
        }
    }
    ret += '/';
    ret += msblenUtils::get_name(obj);
    ret = append_id(ret, obj);
    return ret;
}

std::string msblenContextIntermediatePathProvider::get_path_with_suffix(const Object* arm, const Bone* obj) {
    std::string ret;
    if (obj->parent)
        ret += get_path_with_suffix(arm, obj->parent);
    else
        ret += get_path_with_suffix(arm);
    ret += '/';
    ret += msblenUtils::get_name(obj);
    ret = append_id(ret, arm);
    return ret;
}
