#pragma once
#include <DNA_node_types.h>
#include "BlenderSyncSettings.h"
#include "MeshSync/SceneGraph/msTexture.h"
#include "MeshSync/Utility/msMaterialExt.h"
#include "MeshSyncClient/msTextureManager.h"

namespace blender {
class msblenMaterialsExportHelper
{
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
	void exportPackedImages(ms::TextureType& textureType, std::function<void(int textureId)> setTextureHandler,
	                        Image* img) const;
	void exportImageFromImageNode(ms::TextureType& textureType, std::function<void(int textureId)> setTextureHandler,
	                              bNode* sourceNode) const;
	void handleImageNodeWithAssignedImage(ms::TextureType& textureType, bool resetIfInputIsTexture,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler, bNode* sourceNode) const;
	void handleImageNode(ms::TextureType& textureType, bool resetIfInputIsTexture,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler, bNode* sourceNode);
	void handleNormalMapNode(const Material* mat, ms::TextureType textureType, bool resetIfInputIsTexture,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler, bNode* sourceNode);
	void handleDisplacementNode(const Material* mat, ms::TextureType textureType, bool resetIfInputIsTexture,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler, bNode* sourceNode);
	void handlePassthrough(const Material* mat, ms::TextureType textureType, bool resetIfInputIsTexture,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler, bNode* sourceNode);
	void handleSocketValue(bNodeSocket* socket,
		std::function<void(const mu::float4& colorValue)> setColorHandler,
		std::function<void(int textureId)> setTextureHandler);

	void exportBasic(const Material* mat, std::shared_ptr<ms::Material> ret);

public:
	BlenderSyncSettings* m_settings;
	ms::TextureManager* m_texture_manager;

	void exportMaterial(const Material* mat, std::shared_ptr<ms::Material> ret);
};
}
