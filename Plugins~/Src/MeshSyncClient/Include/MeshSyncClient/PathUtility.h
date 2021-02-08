#pragma once

#include <string>

namespace MeshSyncClient {

class PathUtility {

public:
    static std::string BuildPathWithExtension(const char* inputPath, const char* ext);
    
};


} // namespace MeshSyncClient
