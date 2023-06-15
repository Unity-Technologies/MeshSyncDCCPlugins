#pragma once
#include "msblenContextIntermediatePathProvider.h"

class msblenContextInstanceChildPathProvider : public msblenContextIntermediatePathProvider {

public:
    std::string get_path(const Object* obj, const Bone* bone) override;
};