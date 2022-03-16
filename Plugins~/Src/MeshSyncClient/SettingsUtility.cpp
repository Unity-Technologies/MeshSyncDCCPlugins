#pragma once

#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h" //SceneCacheOutputSettings


#include "MeshSyncClient/SettingsUtility.h"
#include "MeshSyncClient/BaseCacheSettings.h"
#include "MeshSyncClient/BaseSyncSettings.h"


namespace MeshSyncClient {

void SettingsUtility::ApplyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings* syncSettings) {
    syncSettings->ExportSceneCache = true;
    syncSettings->make_double_sided = cacheSettings.make_double_sided;
    syncSettings->BakeModifiers = cacheSettings.bake_modifiers;
    syncSettings->BakeTransform = cacheSettings.bake_transform;
    syncSettings->flatten_hierarchy = cacheSettings.flatten_hierarchy;
    syncSettings->Validate();
}

//----------------------------------------------------------------------------------------------------------------------

ms::SceneCacheOutputSettings SettingsUtility::CreateSceneCacheOutputSettings(float sampleRate, const BaseCacheSettings& cacheSettings) {
    ms::SceneCacheOutputSettings oscs;
    oscs.exportSettings.sampleRate = sampleRate;
    oscs.exportSettings.encoderSettings.zstd.compressionLevel = cacheSettings.zstd_compression_level;
    oscs.exportSettings.flattenHierarchy = cacheSettings.flatten_hierarchy;
    oscs.exportSettings.stripNormals = cacheSettings.strip_normals;
    oscs.exportSettings.stripTangents = cacheSettings.strip_tangents;
    return oscs;
}


} // namespace MeshSyncClient
