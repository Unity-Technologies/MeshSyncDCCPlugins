#pragma once

#include "MeshSyncClient/PathUtility.h"
#include "Poco/Path.h"

namespace MeshSyncClient {

std::string PathUtility::BuildPathWithExtension(const char* inputPath, const char* ext) {
    Poco::Path path(inputPath);
    path.setExtension(ext);
    return path.toString();
}


} // namespace MeshSyncClient

