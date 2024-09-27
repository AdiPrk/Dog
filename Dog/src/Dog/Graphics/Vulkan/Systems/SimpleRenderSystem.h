#pragma once

#include "../Camera.h"
#include "../Core/Device.h"
#include "../FrameInfo.h"
#include "Entities/GameObject.h"
#include "../Pipeline/Pipeline.h"
#include "../Texture/TextureLibrary.h"
#include "../Models/ModelLibrary.h"

namespace Dog {

    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(
            Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, TextureLibrary& textureLibrary, ModelLibrary& modelLibrary);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderGameObjects(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device& device;
        TextureLibrary& textureLibrary;
        ModelLibrary& modelLibrary;

        std::unique_ptr<Pipeline> lvePipeline;
        std::unique_ptr<Pipeline> lveWireframePipeline;
        VkPipelineLayout pipelineLayout;
    };

} // namespace Dog
