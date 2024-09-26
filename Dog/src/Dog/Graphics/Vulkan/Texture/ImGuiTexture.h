#pragma once

// forward declarations
class LveDevice;
class LveRenderer;
class TextureLibrary;

class ImGuiTextureManager {
public:
	// needs device, renderer, texture manager
	ImGuiTextureManager(LveDevice& device, LveRenderer& renderer);
	~ImGuiTextureManager();

	VkDescriptorSet CreateDescriptorSet(const VkImageView& imageView, const VkSampler& sampler);

	void AddTexture(const std::string& texturePath, const VkImageView& imageView, const VkSampler& sampler);

	// get descriptor set
	VkDescriptorSet GetDescriptorSet(const std::string& texturePath);


private:
	LveDevice& lveDevice;
	LveRenderer& lveRenderer;

	std::unordered_map<std::string, VkDescriptorSet> descriptorMap;
};
