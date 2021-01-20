#pragma once

#include "MeshSyncClient/BaseSyncSettings.h"

struct BlenderSyncSettings : public MeshSyncClient::BaseSyncSettings {
    ms::SceneSettings scene_settings;
    bool curves_as_mesh = true;
    bool calc_per_index_normals = true;
    int frame_step = 1;
    bool multithreaded = true;
    void validate();
};

