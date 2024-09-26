#pragma once

#include "../Camera.h"
#include "../Core/Device.h"
#include "../FrameInfo.h"
#include "Entities/GameObject.h"
#include "../Pipeline/Pipeline.h"

class PointLightSystem {
public:
    PointLightSystem(
        LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem&) = delete;
    PointLightSystem& operator=(const PointLightSystem&) = delete;

    void update(FrameInfo& frameInfo, GlobalUbo& ubo);
    void render(FrameInfo& frameInfo);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    LveDevice& lveDevice;

    std::unique_ptr<Pipeline> lvePipeline;
    VkPipelineLayout pipelineLayout;
};