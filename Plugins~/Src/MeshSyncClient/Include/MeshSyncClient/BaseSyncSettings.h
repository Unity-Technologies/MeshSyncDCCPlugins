#pragma once

#include "MeshSync/msClientSettings.h" //ClientSettings

namespace MeshSyncClient {

struct BaseSyncSettings {

    ms::ClientSettings client_settings;

    bool is_setup = false;

    bool sync_meshes = true;
    bool sync_normals = true;
    bool sync_uvs = true;
    bool sync_colors = true;

    bool sync_blendshapes = true;
    bool sync_bones = true;
    bool sync_textures = true;
    bool sync_cameras = true;
    bool sync_lights = true;

    bool make_double_sided = false;    
    bool BakeModifiers = false; 
    bool BakeTransform = false;

    bool flatten_hierarchy = false;

    // cache. [TODO-sin: 2021-2-8] Remove this flag if possible
    bool ExportSceneCache = false;

    void Validate();

};

} // namespace MeshSyncClient
