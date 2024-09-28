#include <PCH/pch.h>
#include "ImGuiTexture.h"
#include "../Core/Device.h"
#include "TextureLibrary.h"
#include "../Renderer.h"
#include "Engine.h"
#include "Graphics/Editor/Editor.h"

namespace Dog {

    ImGuiTextureManager::ImGuiTextureManager(Device& device)
        : device(device)
    {
    }

    ImGuiTextureManager::~ImGuiTextureManager()
    {
    }

    VkDescriptorSet ImGuiTextureManager::CreateDescriptorSet(const VkImageView& imageView, const VkSampler& sampler) {
        VkDescriptorSet descriptorSet;

        // Allocate the descriptor set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = Engine::Get().GetEditor().imGuiDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &Engine::Get().GetEditor().samplerSetLayout;

        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set!");
        }

        // Get the texture information
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        // Write the descriptor set with the texture info
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;  // Binding matches layout binding
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

        return descriptorSet;
    }

    void ImGuiTextureManager::AddTexture(const std::string& texturePath, const VkImageView& imageView, const VkSampler& sampler)
    {
        descriptorMap[texturePath] = CreateDescriptorSet(imageView, sampler);
    }

    VkDescriptorSet ImGuiTextureManager::GetDescriptorSet(const std::string& texturePath)
    {
        // check if in, otherwise return nullptr
        if (descriptorMap.find(texturePath) == descriptorMap.end())
        {
            throw std::runtime_error("Texture not found in descriptor map!");
        }

        return descriptorMap[texturePath];
    }

} // namespace Dog