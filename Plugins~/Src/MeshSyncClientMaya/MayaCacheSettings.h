#pragma once

#include "MeshSyncClient/BaseCacheSettings.h"

struct MayaCacheSettings : public MeshSyncClient::BaseCacheSettings {
    bool remove_namespace = true;
};

