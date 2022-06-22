#include "msblenContextIntermediatePathProvider.h"
#include "msblenUtils.h"
#include <BLI_listbase.h>

std::string msblenContextIntermediatePathProvider::append_id(std::string path, const Object* obj) {
    auto data = (ID*)obj->data;
    
    path += "_" + std::string(data->name);

    // If we already have an object with this name but a different session_uuid, append the session_uuid as well
    auto it = mappedNames.find(data->name);
    
    if (it == mappedNames.end()) {
        mappedNames.insert(std::make_pair(data->name, data->session_uuid));
    }
    else {
        bool found = false;
        for (; it != mappedNames.end(); it++)
        {
            if (it->second == data->session_uuid) {
                found = true;
                break;
            }
        }

        if (!found) {
            path += "_" + std::to_string(data->session_uuid);
            mappedNames.insert(std::make_pair(data->name, data->session_uuid));
        }
    }

    return path;
}

std::string msblenContextIntermediatePathProvider::get_path(const Object* obj, const Bone* bone)
{
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
