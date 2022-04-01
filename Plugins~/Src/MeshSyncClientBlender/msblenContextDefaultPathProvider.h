#pragma once
#include <string>
#include <pch.h>
#include <msblenContextPathProvider.h>
#include <msblenUtils.h>

class msblenContextDefaultPathProvider : public msblenContextPathProvider {
public:
	std::string get_path(const Object* obj);
	std::string get_path(const Object* arm, const Bone* obj);


};