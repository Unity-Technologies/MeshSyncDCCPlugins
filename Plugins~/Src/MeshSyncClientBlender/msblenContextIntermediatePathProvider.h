#pragma once
#include <msblenContextPathProvider.h>

class msblenContextIntermediatePathProvider : public msblenContextPathProvider {
protected:
    std::string append_id(std::string path, const Object* obj);

    std::string get_path_with_suffix(const Object* obj);
    std::string get_path_with_suffix(const Object* arm, const Bone* obj);
public: 
    std::string get_path(const Object* obj, const Bone* bone) override;

    std::map<std::string, unsigned int> mappedNames;
};
