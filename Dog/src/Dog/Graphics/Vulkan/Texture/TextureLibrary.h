#pragma once

#include "Texture.h"
#include "ImGuiTexture.h"

class LveRenderer;

class TextureLibrary {
public:
	TextureLibrary(LveDevice& device, LveRenderer& renderer);
	~TextureLibrary();

	TextureLibrary(const TextureLibrary&) = delete;
	TextureLibrary& operator=(const TextureLibrary&) = delete;

	const static uint32_t INVALID_TEXTURE_INDEX;

	uint32_t AddTexture(const std::string& texturePath);
	uint32_t AddTextureFromMemory(const unsigned char* textureData, int textureSize);

	uint32_t GetTexture(const std::string& texturePath);

	VkDescriptorSet GetDescriptorSet(const std::string& texturePath);

	Texture& getTextureByIndex(const size_t& index) { return *textures[index]; }
	VkDescriptorSet GetDescriptorSetByIndex(const size_t& index);

	const size_t getTextureCount() const { return textures.size(); }

private:
	std::vector<std::unique_ptr<Texture>> textures;
	std::unordered_map<std::string, uint32_t> textureMap;
	LveDevice& lveDevice;
	LveRenderer& renderer;

	ImGuiTextureManager imGuiTextureManager;

	uint32_t bakedInTextureCount = 0;
};
