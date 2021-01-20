#pragma once

#include "MeshSyncClient/BaseCacheSettings.h"

struct MQCacheSettings : public MeshSyncClient::BaseCacheSettings {
    mu::nanosec time_start = 0;
};

