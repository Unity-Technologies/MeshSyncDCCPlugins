#pragma once

#include "MeshSync/SceneGraph/msSceneSettings.h" //SceneSettings
#include "MeshSyncClient/BaseSyncSettings.h"

struct BlenderSyncSettings : public MeshSyncClient::BaseSyncSettings {
    ms::SceneSettings scene_settings;
    bool curves_as_mesh = true;
    bool calc_per_index_normals = true;
    int frame_step = 1;
    bool multithreaded = true;

    // Keep in sync with unity_mesh_sync_common meshsync_material_sync_mode.py EnumProperty items:
    enum class MaterialSyncMode
    {
        None  = 0,
        Basic = 1
    };

    int material_sync_mode = 0;
};

