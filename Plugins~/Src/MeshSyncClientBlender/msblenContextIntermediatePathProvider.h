#pragma once
#include <msblenContextPathProvider.h>

class msblenContextIntermediatePathProvider: public msblenContextPathProvider {
public: 
	std::string get_path(const Object* obj);
	std::string get_path(const Object* obj, const Bone* bone);
};