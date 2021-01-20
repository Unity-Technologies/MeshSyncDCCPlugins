#pragma once

#include "MeshSyncClient/BaseSyncSettings.h"

struct ModoSyncSettings : public MeshSyncClient::BaseSyncSettings {
    float scale_factor = 1.0f;
    float frame_step = 1.0f;
    int  timeout_ms = 5000;
    bool auto_sync = false;

    bool sync_mesh_instances = true;
    bool sync_replicators = true;

    void validate();
};

