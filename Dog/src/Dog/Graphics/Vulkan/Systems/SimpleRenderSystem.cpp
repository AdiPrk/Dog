#include <PCH/pch.h>
#include "SimpleRenderSystem.h"

namespace Dog {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
        int textureIndex{ 0 };
    };

    SimpleRenderSystem::SimpleRenderSystem(
        Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, TextureLibrary& textureLibrary, ModelLibrary& modelLibrary)
        : device{ device }
        , textureLibrary{ textureLibrary }
        , modelLibrary{ modelLibrary }
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
        lvePipeline = std::make_unique<Pipeline>(
            device,
            "simple_shader.vert",
            "simple_shader.frag",
            pipelineConfig);

        // Using base pipeline is supposed to make pipeline creation more efficient
        // It should also make switching between the two pipelines more efficient
        pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
        pipelineConfig.rasterizationInfo.lineWidth = 1.0f;
        pipelineConfig.basePipelineIndex = -1; // -1 forces it to use basePipelineHandle (which must be valid)
        pipelineConfig.basePipelineHandle = lvePipeline->getPipeline();
        pipelineConfig.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        lveWireframePipeline = std::make_unique<Pipeline>(
            device,
            "simple_shader.vert",
            "simple_shader.frag",
            pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {

        static float frame = 0;
        static bool wireframe = false;
        frame += frameInfo.frameTime;
        if (frame > 3.f) {
            frame = 0;
            wireframe = !wireframe;
        }

        if (wireframe) {
            //lveWireframePipeline->bind(frameInfo.commandBuffer);
        }
        else {
        }
        lvePipeline->bind(frameInfo.commandBuffer);

        //lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;

            Model* model;

            if ((model = modelLibrary.GetModelByIndex(obj.model)) == nullptr) {
                continue;
            }

            for (auto& mesh : model->meshes) {
                SimplePushConstantData push{};
                push.modelMatrix = obj.transform.mat4();
                push.normalMatrix = obj.transform.normalMatrix();

                if (mesh.textureIndex == 999 && obj.textureIndex != 999) {
                    push.textureIndex = obj.textureIndex;
                }
                else {
                    push.textureIndex = mesh.textureIndex;
                }

                vkCmdPushConstants(
                    frameInfo.commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push);

                mesh.bind(frameInfo.commandBuffer);
                mesh.draw(frameInfo.commandBuffer);
            }
        }
    }

} // namespace Dog