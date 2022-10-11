#include "msblenMaterialsExportHelper.h"

#include <BKE_node.h>
#include <BLI_utildefines.h>

#include "msblenUtils.h"
#include "MeshSync/SceneGraph/msTexture.h"
#include "MeshSync/Utility/msMaterialExt.h"

namespace blender {
// Blender uses hardcoded string identifiers to figure out what the sockets do:
const auto baseColorIdentifier = "Base Color";
const auto colorIdentifier = "Color";
const auto roughnessIdentifier = "Roughness";
const auto metallicIdentifier = "Metallic";
const auto normalIdentifier = "Normal";
const auto normalStrengthIdentifier = "Strength";
const auto surfaceIdentifier = "Surface";
const auto emissionIdentifier = "Emission";
const auto emissionStrengthIdentifier = "Emission Strength";

const auto shaderIdentifier = "Shader";

const auto displacementIdentifier = "Displacement";
const auto heightIdentifier = "Height";
const auto scaleIdentifier = "Scale";

bNode* removeReroutes(bNode* node, const Material* mat) {
	if (!node || node->type != NODE_REROUTE) {
		return node;
	}

	auto tree = mat->nodetree;

	for (auto link : list_range((bNodeLink*)tree->links.first)) {
		if (link->tonode == node) {
			return removeReroutes(link->fromnode, mat);
		}
	}

	return nullptr;
}

bNode* getNodeConnectedToSocket(bNodeSocket* socket) {
	if (socket->link) {
		return socket->link->fromnode;
	}

	return nullptr;
}

// For mix shaders, we pass through the first input that has a connection.
bNode* handleMixShader(const Material* mat, bNode* bsdf) {
	if (!bsdf || bsdf->type != SH_NODE_MIX_SHADER)
		return bsdf;

	for (auto inputSocket : list_range((bNodeSocket*)bsdf->inputs.first)) {
		if (STREQ(inputSocket->name, shaderIdentifier)) {
			bNode* connectedBSDF = removeReroutes(getNodeConnectedToSocket(inputSocket), mat);
			if (connectedBSDF)
			{
				return connectedBSDF;
			}
		}
	}

	return bsdf;
}

bool getBSDFAndOutput(const Material* mat, bNode*& bsdf, bNode*& output) {
	// Find BSDF connected to an output node:
	auto tree = mat->nodetree;
	for (auto link : list_range((bNodeLink*)tree->links.first)) {
		if (link->tonode &&
			link->tonode->type == SH_NODE_OUTPUT_MATERIAL &&
			STREQ(link->tosock->identifier, surfaceIdentifier)) {
			bsdf = removeReroutes(link->fromnode, mat);

			bsdf = handleMixShader(mat, bsdf);

			output = link->tonode;
			return bsdf && output;
		}
	}

	return false;
}

bNodeSocket* getInputSocket(bNode* node, const char* socketName) {
	for (auto inputSocket : list_range((bNodeSocket*)node->inputs.first)) {
		if (STREQ(inputSocket->name, socketName)) {
			return inputSocket;
		}
	}

	return nullptr;
}

int msblenMaterialsExportHelper::exportTexture(const std::string& path, ms::TextureType type) const
{
	return m_texture_manager->addFile(path, type);
}

void msblenMaterialsExportHelper::handleImageNodeWithAssignedImage(ms::TextureType& textureType,
	bool resetIfInputIsTexture,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler,
	bNode* sourceNode)
{
	if (setTextureHandler) {
		auto img = (Image*)sourceNode->id;

		// Use non-color if the image node is not in sRGB space:
		if (textureType == ms::TextureType::Default && !STREQ(img->colorspace_settings.name, "sRGB")) {
			textureType = ms::TextureType::NonColor;
		}

		// Unpack if needed:
		if (img->packedfiles.first)
		{
			for (auto imagePackedFile : list_range((ImagePackedFile*)img->packedfiles.first)) {
				std::string name = img->id.name + 2; // +2 because of IM prefix!

				// Use extension from filename if the name in the image node has a different extension:
				std::string filepath = img->filepath;
				size_t extensionIndex = filepath.find_last_of('.');
				if (extensionIndex > 0) {
					std::string extension = filepath.substr(extensionIndex);

					if (name.find(extension) != name.length() - extension.length()) {
						name += extension;
					}
				}

				int exported = m_texture_manager->addImage(name, 0, 0, imagePackedFile->packedfile->data, imagePackedFile->packedfile->size, ms::TextureFormat::RawFile, textureType);
				setTextureHandler(exported);
			}
		}
		else {
			setTextureHandler(exportTexture(abspath(img->filepath), textureType));
		}
	}

	// Ensure the color is white if there is a texture otherwise Unity will multiply that image with the color that was set before.
	// Blender does not do that so they would not look the same:
	if (resetIfInputIsTexture && setColorHandler) {
		setColorHandler(mu::float4{ 1, 1, 1, 1 });
	}
}

void msblenMaterialsExportHelper::handleImageNode(ms::TextureType& textureType,
	bool resetIfInputIsTexture,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler,
	bNode* sourceNode)
{
	if (sourceNode->id) {
		handleImageNodeWithAssignedImage(textureType, resetIfInputIsTexture, setColorHandler, setTextureHandler, sourceNode);
	}
	else {
		if (setTextureHandler) {
			setTextureHandler(ms::InvalidID);
		}

		if (setColorHandler) {
			// Blender uses black if there is no image set on an image node:
			setColorHandler(mu::float4{ 0, 0, 0, 1 });
		}
	}
}

void msblenMaterialsExportHelper::handleNormalMapNode(const Material* mat,
	ms::TextureType textureType,
	bool resetIfInputIsTexture,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler,
	bNode* sourceNode)
{
	if (setTextureHandler) {
		auto imageInput = getInputSocket(sourceNode, colorIdentifier);
		if (imageInput)
		{
			setValueFromSocket(mat,
				imageInput, textureType,
				resetIfInputIsTexture,
				nullptr,
				setTextureHandler);
		}
	}

	auto strengthInput = getInputSocket(sourceNode, normalStrengthIdentifier);
	if (strengthInput && setColorHandler) {
		auto defaultValue = (bNodeSocketValueFloat*)strengthInput->default_value;
		auto val = defaultValue->value;
		setColorHandler(mu::float4{ val,val,val, val });
	}
}

void msblenMaterialsExportHelper::handleDisplacementNode(const Material* mat,
	ms::TextureType textureType,
	bool resetIfInputIsTexture,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler,
	bNode* sourceNode)
{
	auto heightInput = getInputSocket(sourceNode, heightIdentifier);
	if (heightInput && setTextureHandler)
	{
		setValueFromSocket(mat,
			heightInput, textureType,
			resetIfInputIsTexture,
			setColorHandler,
			setTextureHandler);
	}

	auto scaleInput = getInputSocket(sourceNode, scaleIdentifier);
	if (scaleInput && setColorHandler)
	{
		auto defaultValue = (bNodeSocketValueFloat*)scaleInput->default_value;
		auto val = defaultValue->value;
		setColorHandler(mu::float4{ val,val,val, val });
	}
}

void msblenMaterialsExportHelper::handlePassthrough(const Material* mat,
	ms::TextureType textureType,
	bool resetIfInputIsTexture,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler,
	bNode* sourceNode)
{
	auto imageInput = getInputSocket(sourceNode, colorIdentifier);
	if (!imageInput)
		return;

	setValueFromSocket(mat,
		imageInput, textureType,
		resetIfInputIsTexture,
		setColorHandler,
		setTextureHandler);
}

void msblenMaterialsExportHelper::handleSocketValue(bNodeSocket* socket,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler)
{
	// The socket is using a direct value, not the output of another node.
	// Clear any texture that was set:
	if (setTextureHandler) {
		setTextureHandler(ms::InvalidID);
	}

	if (setColorHandler) {
		mu::float4 colorValue;

		switch (socket->type)
		{
		case SOCK_RGBA: {
			auto defaultValue = (bNodeSocketValueRGBA*)socket->default_value;
			auto val = defaultValue->value;
			colorValue = mu::float4{ val[0], val[1], val[2], val[3] };
			break;
		}
		case SOCK_FLOAT: {
			auto defaultValue = (bNodeSocketValueFloat*)socket->default_value;
			auto val = defaultValue->value;
			colorValue = mu::float4{ val, val, val, val };
			break;
		}
		}
		setColorHandler(colorValue);
	}
}

void msblenMaterialsExportHelper::setValueFromSocket(const Material* mat,
	bNodeSocket* socket,
	ms::TextureType textureType,
	bool resetIfInputIsTexture,
	std::function<void(const mu::float4& colorValue)> setColorHandler,
	std::function<void(int textureId)> setTextureHandler)
{
	// If there is nothing connected to the socket, send the value set on the socket:
	if (!socket->link) {
		handleSocketValue(socket, setColorHandler, setTextureHandler);
		return;
	}

	// If there is an image linked to the socket, send that as a texture:
	auto sourceNode = removeReroutes(socket->link->fromnode, mat);

	if (sourceNode->flag & NODE_MUTED) {
		return;
	}

	if (!m_settings->sync_textures) {
		setTextureHandler = nullptr;
	}

	switch (sourceNode->type) {
	case SH_NODE_TEX_IMAGE:
	{
		handleImageNode(textureType, resetIfInputIsTexture, setColorHandler, setTextureHandler, sourceNode);
		break;
	}
	case SH_NODE_NORMAL_MAP:
	{
		handleNormalMapNode(mat, textureType, resetIfInputIsTexture, setColorHandler, setTextureHandler, sourceNode);
		break;
	}
	case SH_NODE_DISPLACEMENT:
	{
		handleDisplacementNode(mat, textureType, resetIfInputIsTexture, setColorHandler, setTextureHandler, sourceNode);
		break;
	}
	// Pass input straight through these:
	case SH_NODE_GAMMA:
	case SH_NODE_HUE_SAT:
	{
		handlePassthrough(mat, textureType, resetIfInputIsTexture, setColorHandler, setTextureHandler, sourceNode);
		break;
	}
	}
}

void msblenMaterialsExportHelper::setShaderFromBSDF(ms::StandardMaterial& stdmat, bNode* bsdfNode)
{
	switch (bsdfNode->type)
	{
	case SH_NODE_BSDF_GLASS:
		stdmat.setShader("Glass");
		break;
	default:
		stdmat.setShader("Default");
		break;
	}
}

void msblenMaterialsExportHelper::setHeightFromOutputNode(const Material* mat, ms::StandardMaterial& stdmat, bNode* outputNode)
{
	auto displacementSocket = getInputSocket(outputNode, displacementIdentifier);
	if (!displacementSocket)
		return;

	setValueFromSocket(mat,
		displacementSocket, ms::TextureType::Default,
		false,
		[&](const mu::float4& colorValue) {
			stdmat.setHeightScale(colorValue[0]);
		},
		[&](int textureId)
		{
			stdmat.setHeightMap(textureId);
		});
}

void msblenMaterialsExportHelper::setPropertiesFromBSDF(const Material* mat, ms::StandardMaterial& stdmat, bNode* bsdfNode)
{
#define isSocket(NAME) STREQ(inputSocket->identifier, NAME)

	// Go through all inputs on the bsdf and find colors and textures to set:
	for (auto inputSocket : list_range((bNodeSocket*)bsdfNode->inputs.first)) {
		if (isSocket(baseColorIdentifier) ||
			isSocket(colorIdentifier)) {
			setValueFromSocket(mat,
				inputSocket, ms::TextureType::Default,
				true,
				[&](const mu::float4& colorValue)
				{
					stdmat.setColor(colorValue);
				},
				[&](int textureId)
				{
					stdmat.setColorMap(textureId);
				});
		}
		else if (isSocket(roughnessIdentifier))
		{
			setValueFromSocket(mat,
				inputSocket, ms::TextureType::Default,
				false,
				[&](const mu::float4& colorValue)
				{
					stdmat.setSmoothness(1 - colorValue[0]);
				},
				[&](int textureId) {
					stdmat.setSmoothnessMap(textureId);
				});
		}
		else if (isSocket(metallicIdentifier))
		{
			setValueFromSocket(mat,
				inputSocket, ms::TextureType::Default,
				true,
				[&](const mu::float4& colorValue) {
					stdmat.setMetallic(colorValue[0]);
				},
				[&](int textureId) {
					stdmat.setMetallicMap(textureId);
				});
		}
		else if (isSocket(normalIdentifier))
		{
			setValueFromSocket(mat,
				inputSocket, ms::TextureType::NormalMap,
				true,
				[&](const mu::float4& colorValue) {
					stdmat.setBumpScale(colorValue[0]);
				},
				[&](int textureId) {
					stdmat.setBumpMap(textureId);
				});
		}
		else if (isSocket(emissionIdentifier))
		{
			setValueFromSocket(mat,
				inputSocket, ms::TextureType::Default,
				true,
				[&](const mu::float4& colorValue) {
					stdmat.setEmissionColor(colorValue);
				},
				[&](int textureId) {
					stdmat.setEmissionMap(textureId);
				});
		}
		else if (isSocket(emissionStrengthIdentifier))
		{
			setValueFromSocket(mat,
				inputSocket, ms::TextureType::Default,
				true,
				[&](const mu::float4& colorValue) {
					// blender emission strength is in W/m^2
					// 1 W/m^2 = 683 Lumen/m^2 (Lux)
					stdmat.setEmissionStrength(colorValue[0] * 683);
				},
				nullptr);
		}
	}
}

void msblenMaterialsExportHelper::exportMaterialFromNodeTree(const Material* mat, ms::StandardMaterial& stdmat)
{
	bNode* bsdfNode;
	bNode* outputNode;

	if (!getBSDFAndOutput(mat, bsdfNode, outputNode)) {
		return;
	}

	setShaderFromBSDF(stdmat, bsdfNode);
	setPropertiesFromBSDF(mat, stdmat, bsdfNode);
	setHeightFromOutputNode(mat, stdmat, outputNode);
}

void msblenMaterialsExportHelper::exportBasic(const Material* mat, std::shared_ptr<ms::Material> ret)
{
	ms::StandardMaterial& stdmat = AsStandardMaterial(*ret);
	BMaterial bm(mat);

	if (mat->use_nodes) {
		exportMaterialFromNodeTree(mat, stdmat);
	}
	else {
		stdmat.setColor(mu::float4{ mat->r, mat->g, mat->b, 1.0f });
		stdmat.setColorMap(nullptr);

		stdmat.setMetallic(mat->metallic);
		stdmat.setSmoothness(1 - mat->roughness);
		stdmat.setSpecular(mu::float3{ mat->spec, mat->spec, mat->spec });
	}
}

void msblenMaterialsExportHelper::exportMaterial(const Material* mat, std::shared_ptr<ms::Material> ret)
{
	switch (m_settings->material_sync_mode)
	{
	case BlenderSyncSettings::MaterialSyncMode::Basic:
		exportBasic(mat, ret);
		break;
	default:
		break;
	}
}
}