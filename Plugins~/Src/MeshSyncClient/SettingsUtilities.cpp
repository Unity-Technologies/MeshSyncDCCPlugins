#pragma once

#include "MeshSync/SceneCache/msSceneCacheSettings.h" //OSceneCacheSettings


#include "MeshSyncClient/SettingsUtilities.h"
#include "MeshSyncClient/BaseCacheSettings.h"
#include "MeshSyncClient/BaseSyncSettings.h"


namespace MeshSyncClient {

void SettingsUtilities::ApplyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings& syncSettings) {
    syncSettings.ExportSceneCache = true;
    syncSettings.make_double_sided = cacheSettings.make_double_sided;
    syncSettings.BakeModifiers = cacheSettings.bake_modifiers;
    syncSettings.BakeTransform = cacheSettings.bake_transform;
    syncSettings.flatten_hierarchy = cacheSettings.flatten_hierarchy;
    syncSettings.Validate();
}

//----------------------------------------------------------------------------------------------------------------------

ms::OSceneCacheSettings SettingsUtilities::CreateOSceneCacheSettings(float sampleRate, const BaseCacheSettings& cacheSettings) {
    ms::OSceneCacheSettings oscs;
    oscs.sample_rate = sampleRate;
    oscs.encoder_settings.zstd.compression_level = cacheSettings.zstd_compression_level;
    oscs.flatten_hierarchy = cacheSettings.flatten_hierarchy;
    oscs.strip_normals = cacheSettings.strip_normals;
    oscs.strip_tangents = cacheSettings.strip_tangents;
    return oscs;
}


} // namespace MeshSyncClient
