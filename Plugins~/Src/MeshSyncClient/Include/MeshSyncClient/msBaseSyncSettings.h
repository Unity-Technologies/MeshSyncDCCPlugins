#pragma once

#include "MeshSync/msClient.h"

namespace ms {

struct msBaseSyncSettings {

    ms::ClientSettings clientSettings;

    bool syncMeshes = true;
    bool syncNormals = true;
    bool syncUVs = true;
    bool syncColors = true;


    bool syncBlendShapes = true;
    bool syncBones = true;
    bool syncTextures = true;
    bool syncCameras = true;
    bool syncLights = true;

    bool makeDoubleSided = false;    
    bool bakeModifiers = false;
    bool bakeTransform = false;

    bool flattenHierarchy = false;

    bool exportSceneCache = false;

};

} // namespace ms
