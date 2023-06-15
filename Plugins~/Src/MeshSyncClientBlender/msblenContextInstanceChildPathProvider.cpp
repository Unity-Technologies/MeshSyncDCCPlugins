#include "msblenContextIntermediatePathProvider.h"
#include "msblenContextInstanceChildPathProvider.h"

std::string msblenContextInstanceChildPathProvider::get_path(const Object* obj, const Bone* bone) {
    std::string path;
    if (bone == nullptr) {
        path = get_path_with_suffix(obj);
    }
    else {
        path = get_path_with_suffix(obj, bone);
    }

    return path;
}