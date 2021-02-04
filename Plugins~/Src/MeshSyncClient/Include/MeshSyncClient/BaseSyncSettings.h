#pragma once

#include <string> //[TODO-sin: 2021-2-4] Remove this. This is required temporarily for std::string in ClientSettings
#include "MeshSync/msClientSettings.h" //ClientSettings

namespace MeshSyncClient {

struct BaseSyncSettings {

    ms::ClientSettings client_settings;

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

    // cache
    bool ExportSceneCache = false;

    void Validate();

};

} // namespace MeshSyncClient
