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
    static void ApplyBMeshUVToMesh(const blender::BMesh* bMesh, const size_t numIndices, ms::Mesh* dest);
    static Material** GetMaterials(Object* obj);
    static short GetNumMaterials(Object* obj);
};

} //end namespace


