#pragma once
#include <msblenContextPathProvider.h>

#include "msblenUtils.h"

class msblenContextIntermediatePathProvider : public msblenContextPathProvider {
protected:
    std::string append_id(std::string path, const Object* obj);

    std::string get_path_with_suffix(const Object* obj);
    std::string get_path_with_suffix(const Object* arm, const Bone* obj);
public: 
    std::string get_path(const Object* obj, const Bone* bone) override;

    std::map<std::string, unsigned int> mappedNames;
};


class msblenContextInstanceChildPathProvider : public msblenContextIntermediatePathProvider {
    //using super = msblenContextIntermediatePathProvider;

public:
   /* msblenContextInstanceChildPathProvider()
    {        
    }*/

  /*  std::string get_path_with_suffix(const Object* obj) {
        std::string ret;
        if (obj->parent) {
            if (obj->partype == PARBONE) {
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
    std::string get_path_with_suffix(const Object* arm, const Bone* obj) {
        std::string ret;
        if (obj->parent)
            ret += get_path_with_suffix(arm, obj->parent);
        else
            ret += get_path_with_suffix(arm);
        ret += '/';
        ret += msblenUtils::get_name(obj);
        ret = append_id(ret, arm);
        return ret;
    }*/

    std::string get_path(const Object* obj, const Bone* bone) override
    {
        std::string path;
        if (bone == nullptr) {
            path = get_path_with_suffix(obj);
        }
        else {
            path = get_path_with_suffix(obj, bone);
        }

        return path;
    }
};