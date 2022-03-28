#include "msblenContextDefaultPathProvider.h"
#include "msblenUtils.h"

std::string msblenContextDefaultPathProvider::get_path(const Object* obj)
{
	return msblenUtils::get_path(obj);
}
std::string msblenContextDefaultPathProvider::get_path(const Object* arm, const Bone* obj) {
	return msblenUtils::get_path(arm, obj);
}

