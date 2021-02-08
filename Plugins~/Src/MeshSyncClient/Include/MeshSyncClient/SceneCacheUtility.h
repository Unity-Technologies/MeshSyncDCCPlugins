#pragma once

#include <string>

namespace MeshSyncClient {

class SceneCacheUtility {

public:
    //requestedPath:
    //    c:/Users/file.bin
    //    c:/Users/asset.sc
    //    c:/Users/asset (sc will be automatically appended)
    static std::string BuildFilePath(const std::string& requestedPath);
    static std::string BuildFilePath(const char* requestedPath);

};


} // namespace MeshSyncClient
