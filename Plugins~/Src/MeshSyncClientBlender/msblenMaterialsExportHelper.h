#pragma once
#include <DNA_node_types.h>
#include "BlenderSyncSettings.h"
#include "MeshSync/SceneGraph/msTexture.h"
#include "MeshSync/Utility/msMaterialExt.h"
#include "MeshSyncClient/msTextureManager.h"

namespace blender {
class msblenMaterialsExportHelper
{
private:
    void SetValueFromSocket(const Material* mat,
        bNodeSocket* socket,
        ms::TextureType textureType,
        bool resetIfInputIsTexture,
        std::function<void(mu::float4& colorValue)> setColorHandler,
        std::function<void(int textureId)> setTextureHandler);

    void ExportMaterialFromNodeTree(const Material* mat, ms::StandardMaterial& stdmat);

    int exportTexture(const std::string& path, ms::TextureType type);

    void exportBasic(const Material* mat, std::shared_ptr<ms::Material> ret);

public:
	BlenderSyncSettings* m_settings;
    ms::TextureManager* m_texture_manager;

	void exportMaterial(const Material* mat, std::shared_ptr<ms::Material> ret);
};
}
 