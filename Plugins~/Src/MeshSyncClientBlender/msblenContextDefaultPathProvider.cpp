#include "msblenContextDefaultPathProvider.h"
#include "msblenUtils.h"

std::string msblenContextDefaultPathProvider::get_path(const Object* arm, const Bone* obj) {
	if (obj == nullptr) {
		return msblenUtils::get_path(arm);
	}

	return msblenUtils::get_path(arm, obj);
}

