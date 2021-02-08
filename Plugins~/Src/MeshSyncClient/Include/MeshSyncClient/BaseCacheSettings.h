#pragma once

#include <string>

#include "MeshSyncClient/FrameRange.h"
#include "MeshSyncClient/MaterialFrameRange.h"
#include "MeshSyncClient/ObjectScope.h"


namespace MeshSyncClient {

struct BaseCacheSettings {

    int zstd_compression_level = 3; // (min) 0 - 22 (max)

    //To export SceneCache ?
    MeshSyncClient::ObjectScope object_scope = MeshSyncClient::ObjectScope::All;
    MeshSyncClient::FrameRange frame_range = MeshSyncClient::FrameRange::All;
    int frame_begin = 0;
    int frame_end = 100;
    float frame_step = 1.0f;
    MeshSyncClient::MaterialFrameRange material_frame_range = MeshSyncClient::MaterialFrameRange::One;

    bool make_double_sided = false;
    bool bake_modifiers = true; 
    bool bake_transform = true;

    bool flatten_hierarchy = false;
    bool merge_meshes = false;

    bool strip_normals = false;
    bool strip_tangents = true;

};

} // namespace MeshSyncClient
