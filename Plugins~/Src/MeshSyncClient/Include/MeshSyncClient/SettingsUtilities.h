#pragma once

namespace ms {
    struct OSceneCacheSettings;
}

namespace MeshSyncClient {

struct BaseCacheSettings;
struct BaseSyncSettings;

class SettingsUtilities {

public:
    static void CopyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings& syncSettings);
    static void CopyCacheToOutputSettings(float sampleRate, const BaseCacheSettings& cacheSettings, 
                                          ms::OSceneCacheSettings& outputSettings);

    
};


} // namespace MeshSyncClient
