#pragma once

#include "../Camera.h"
#include "../Core/Device.h"
#include "../FrameInfo.h"
#include "Entities/GameObject.h"
#include "../Pipeline/Pipeline.h"
#include "../Texture/TextureLibrary.h"

class SimpleRenderSystem {
public:
    SimpleRenderSystem(
        LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, TextureLibrary& textureLibrary);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

    void renderGameObjects(FrameInfo& frameInfo);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    LveDevice& lveDevice;
    TextureLibrary& textureLibrary;

    std::unique_ptr<Pipeline> lvePipeline;
    std::unique_ptr<Pipeline> lveWireframePipeline;
    VkPipelineLayout pipelineLayout;
};