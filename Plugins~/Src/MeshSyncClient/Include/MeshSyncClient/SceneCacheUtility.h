#pragma once

#include <string>

namespace MeshSyncClient {

class SceneCacheUtility {

public:
    //requestedPath: c:/Users/file.txt
    static std::string BuildFilePath(const char* requestedPath);

};


} // namespace MeshSyncClient
