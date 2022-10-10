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
	void setValueFromSocket(const Material* mat,
		bNodeSocket* socket,
		ms::TextureType textureType,
		bool resetIfInputIsTexture,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler);
	void setShaderFromBSDF(ms::StandardMaterial& stdmat, bNode* bsdfNode);
	void setHeightFromOutputNode(const Material* mat, ms::StandardMaterial& stdmat, bNode* outputNode);
	void setPropertiesFromBSDF(const Material* mat, ms::StandardMaterial& stdmat, bNode* bsdfNode);

	void exportMaterialFromNodeTree(const Material* mat, ms::StandardMaterial& stdmat);

	int exportTexture(const std::string& path, ms::TextureType type) const;

	void exportBasic(const Material* mat, std::shared_ptr<ms::Material> ret);

public:
	BlenderSyncSettings* m_settings;
	ms::TextureManager* m_texture_manager;

	void exportMaterial(const Material* mat, std::shared_ptr<ms::Material> ret);
};
}
