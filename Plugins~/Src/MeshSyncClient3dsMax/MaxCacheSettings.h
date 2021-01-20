#pragma once

#include "MeshSyncClient/BaseCacheSettings.h"

struct MaxCacheSettings : public MeshSyncClient::BaseCacheSettings {
    bool ignore_non_renderable = true;
    bool use_render_meshes = true;
};

