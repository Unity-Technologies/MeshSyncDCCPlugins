#pragma once

#include "MeshSyncClient/BaseCacheSettings.h"

struct BlenderCacheSettings : public MeshSyncClient::BaseCacheSettings {
    bool curves_as_mesh = true;
};

