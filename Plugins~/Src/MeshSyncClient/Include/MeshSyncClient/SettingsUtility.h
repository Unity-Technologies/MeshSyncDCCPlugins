#pragma once

namespace ms {
    struct SceneCacheOutputSettings;
}

namespace MeshSyncClient {

struct BaseCacheSettings;
struct BaseSyncSettings;

class SettingsUtility {

public:
    static void ApplyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings* syncSettings);
    static ms::SceneCacheOutputSettings CreateOSceneCacheSettings(float sampleRate, const BaseCacheSettings& cacheSettings);

    
};


} // namespace MeshSyncClient
