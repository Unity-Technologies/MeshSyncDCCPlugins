#pragma once

#include <string>

namespace MeshSyncClient {

class PathUtility {

public:
    //inputPath: c:/Users/file.txt
    //ext: sc (without dot)
    static std::string BuildPathWithExtension(const char* inputPath, const char* ext);
    
};


} // namespace MeshSyncClient
