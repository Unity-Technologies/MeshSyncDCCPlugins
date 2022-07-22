#pragma once
#include <msblenContextPathProvider.h>

class msblenContextIntermediatePathProvider : public msblenContextPathProvider {
private:
    std::string append_id(std::string path, const Object* obj);

public: 
    std::string get_path(const Object* obj, const Bone* bone) override;

    std::map<std::string, unsigned int> mappedNames;
};