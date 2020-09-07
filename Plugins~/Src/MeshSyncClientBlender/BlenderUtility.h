#pragma once

#include "MeshSync/MeshSync.h"

namespace ms {
class Mesh;
}

//----------------------------------------------------------------------------------------------------------------------

namespace blender{

class BMesh;

class BlenderUtility {
public:
    static void ApplyBlenderMeshUVToMesh(const blender::BMesh* bMesh, const size_t numIndices, ms::Mesh* dest);
};

} //end namespace


