#include <PCH/pch.h>
#include "TextureLibrary.h"

namespace Dog {

	TextureLibrary::TextureLibrary(Device& device, Renderer& renderer)
		: device(device)
		, renderer(renderer)
		, imGuiTextureManager(device, renderer)
		, bakedInTextureCount(0)
	{
	}

	TextureLibrary::~TextureLibrary()
	{
	}

	uint32_t TextureLibrary::AddTexture(const std::string& texturePath) {
		// check size 
		if (textures.size() >= MAX_TEXTURE_COUNT) {
			throw std::runtime_error("Texture count exceeded maximum");
			return INVALID_TEXTURE_INDEX;
		}

		if (textureMap.find(texturePath) == textureMap.end()) {
			textureMap[texturePath] = static_cast<uint32_t>(textures.size());
			textures.push_back(std::make_unique<Texture>(device, texturePath));

			imGuiTextureManager.AddTexture(texturePath, textures.back()->getImageView(), textures.back()->getSampler());

			return static_cast<uint32_t>(textures.size() - 1);
		}
		else {
			return INVALID_TEXTURE_INDEX;
		}
	}

	uint32_t TextureLibrary::AddTextureFromMemory(const unsigned char* textureData, int textureSize)
	{
		// check size 
		if (textures.size() >= MAX_TEXTURE_COUNT) {
			throw std::runtime_error("Texture count exceeded maximum");
			return INVALID_TEXTURE_INDEX;
		}

		std::string newPath = "BAKED_IN_" + std::to_string(bakedInTextureCount);
		bakedInTextureCount++;

		textureMap[newPath] = static_cast<uint32_t>(textures.size());
		textures.push_back(std::make_unique<Texture>(device, newPath, textureData, textureSize));

		imGuiTextureManager.AddTexture(newPath, textures.back()->getImageView(), textures.back()->getSampler());

		return static_cast<uint32_t>(textures.size() - 1);
	}

	uint32_t TextureLibrary::GetTexture(const std::string& texturePath) {
		if (textureMap.find(texturePath) != textureMap.end()) {
			return textureMap[texturePath];
		}
		else {
			return INVALID_TEXTURE_INDEX; // 999 is the 'no texture' id.
		}
	}

	VkDescriptorSet TextureLibrary::GetDescriptorSet(const std::string& texturePath)
	{
		return imGuiTextureManager.GetDescriptorSet(texturePath);
	}

	VkDescriptorSet TextureLibrary::GetDescriptorSetByIndex(const size_t& index)
	{
		return imGuiTextureManager.GetDescriptorSet(
			getTextureByIndex(index).path
		);
	}

} // namespace Dog