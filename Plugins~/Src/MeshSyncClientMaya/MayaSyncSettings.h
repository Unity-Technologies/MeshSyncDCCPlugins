#pragma once

#include "MeshSyncClient/BaseSyncSettings.h"

struct MayaSyncSettings : public MeshSyncClient::BaseSyncSettings {
    float scale_factor = 0.01f;
    float frame_step = 1.0f;
    int  timeout_ms = 5000;
    bool auto_sync = false;
    bool apply_tweak = false;
    bool sync_constraints = false;
    bool remove_namespace = true;
    bool multithreaded = false;
    bool fbx_compatible_transform = true;

    void validate();
};

