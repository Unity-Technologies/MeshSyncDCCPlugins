#pragma once

#include "MeshSyncClient/SceneCacheUtility.h"
#include "Poco/Path.h"

namespace MeshSyncClient {

std::string SceneCacheUtility::BuildFilePath(const char* requestedPath) {
    Poco::Path path(requestedPath);
    if (path.getExtension().length() > 0) {
        return path.toString();
    }

    path.setExtension(/*SCENE_CACHE_EXT = */ "sc");
    return path.toString();
}


} // namespace MeshSyncClient

