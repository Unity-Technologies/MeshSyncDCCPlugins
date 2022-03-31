#include "msblenContextIntermediatePathProvider.h"
#include "msblenUtils.h"
#include <BLI_listbase.h>


std::string append_id(std::string path, const Object* obj) {
    auto data = (ID*)obj->data;
    path += "_" + std::to_string(data->session_uuid);
    return path;
}

std::string get_name_with_id(const Object* obj, const Bone* bone = nullptr) {
    
    std::string path;
    if (bone) {
        path = msblenUtils::get_path(obj, bone);
    }
    else {
        path = "/" + msblenUtils::get_name(obj);
    }

    auto data = (ID*)obj->data;
    auto name = std::string(data->name);

    return append_id(path, obj);
}

std::string msblenContextIntermediatePathProvider::get_path(const Object* obj)
{
    auto path = get_name_with_id(obj);
    return path;
}

std::string msblenContextIntermediatePathProvider::get_path(const Object* obj, const Bone* bone)
{
    auto path = get_name_with_id(obj, bone);
    return path;
}
