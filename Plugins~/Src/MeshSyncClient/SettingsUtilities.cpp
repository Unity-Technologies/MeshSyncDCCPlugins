#pragma once

#include "MeshSyncClient/SettingsUtilities.h"

#include "MeshSyncClient/BaseCacheSettings.h"
#include "MeshSyncClient/BaseSyncSettings.h"


namespace MeshSyncClient {

void SettingsUtilities::CopyCacheToSyncSettings(const BaseCacheSettings& cacheSettings, BaseSyncSettings& syncSettings) {
    syncSettings.ExportSceneCache = true;
    syncSettings.make_double_sided = cacheSettings.make_double_sided;
    syncSettings.BakeModifiers = cacheSettings.bake_modifiers;
    syncSettings.BakeTransform = cacheSettings.bake_transform;
    syncSettings.flatten_hierarchy = cacheSettings.flatten_hierarchy;
    syncSettings.Validate();
}



} // namespace MeshSyncClient
