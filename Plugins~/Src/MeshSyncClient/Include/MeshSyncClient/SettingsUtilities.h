#pragma once

namespace MeshSyncClient {

struct BaseCacheSettings;
struct BaseSyncSettings;

class SettingsUtilities {

public:
    static void CopyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings& syncSettings);
    
};


} // namespace MeshSyncClient
