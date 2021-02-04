#pragma once

#include "MeshSyncClient/BaseSyncSettings.h"

struct MaxSyncSettings : public MeshSyncClient::BaseSyncSettings {
    int timeout_ms = 5000;
    float scale_factor = 1.0f;
    bool auto_sync = false;

    bool flip_faces = true;
    bool use_render_meshes = false;

    bool ignore_non_renderable = true;

    float frame_step = 1.0f;

    // parallel mesh extraction.
    // it seems can cause problems when exporting objects with EvalWorldState()...
    bool multithreaded = false;

};

