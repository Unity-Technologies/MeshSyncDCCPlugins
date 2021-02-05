#pragma once

namespace ms {
    struct OSceneCacheSettings;
}

namespace MeshSyncClient {

struct BaseCacheSettings;
struct BaseSyncSettings;

class SettingsUtilities {

public:
    static void ApplyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings& syncSettings);
    static ms::OSceneCacheSettings CreateOSceneCacheSettings(float sampleRate, const BaseCacheSettings& cacheSettings);

    
};


} // namespace MeshSyncClient
