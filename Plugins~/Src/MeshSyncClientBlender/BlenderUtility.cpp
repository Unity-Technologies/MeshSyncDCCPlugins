#include "pch.h"

#include "BlenderUtility.h"

#include "MeshSync/msConstants.h" //msConstants::MAX_UV
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshUtils/muMath.h" //mu::float2
#include "MeshUtils/muRawVector.h" //SharedVector

#include "msBlenBinder.h" //BMesh

namespace blender {

void BlenderUtility::ApplyBMeshUVToMesh(const blender::BMesh* bMesh, const size_t numIndices, ms::Mesh* dest) {

    for (uint32_t uvIndex=0;uvIndex<ms::msConstants::MAX_UV;++uvIndex) {
        MLoopUV* loopUV = bMesh->GetUV(uvIndex);
        if (nullptr == loopUV)
            continue;

        SharedVector<mu::float2>& curUV = dest->m_uv[uvIndex];
        curUV.resize_discard(numIndices);
        for (size_t ii = 0; ii < numIndices; ++ii) {
            curUV[ii] = (mu::float2&)loopUV->uv;
            ++loopUV;
        }
    }

}


} //end namespace


