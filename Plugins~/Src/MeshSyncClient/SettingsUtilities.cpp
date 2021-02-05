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

void SettingsUtilities::ApplyCacheToOutputSettings(float sampleRate, const BaseCacheSettings& cacheSettings, 
                                                  ms::OSceneCacheSettings& outputSettings)
{
    outputSettings.sample_rate = sampleRate;
    outputSettings.encoder_settings.zstd.compression_level = cacheSettings.zstd_compression_level;
    outputSettings.flatten_hierarchy = cacheSettings.flatten_hierarchy;
    outputSettings.strip_normals = cacheSettings.strip_normals;
    outputSettings.strip_tangents = cacheSettings.strip_tangents;
}




} // namespace MeshSyncClient
