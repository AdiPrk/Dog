#pragma once

#include "Core/Device.h"
#include "Core/SwapChain.h"
#include "Window/Window.h"
#include "Entities/GameObject.h"

namespace Dog {

    class TextureLibrary;
    class ModelLibrary;
    class SimpleRenderSystem;
    class PointLightSystem;
    class DescriptorPool;
    class KeyboardMovementController;

    class Renderer {
    public:
        Renderer(Window& window, Device& device);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void Init();
        void Render(float dt, GameObject::Map& gameObjects);
        void Exit();

        VkRenderPass getSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); }
        float getAspectRatio() const { return m_SwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        // get swapchain
        SwapChain& GetSwapChain() { return *m_SwapChain; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        Window& m_Window;
        Device& device;
        std::unique_ptr<SwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{ 0 };
        bool isFrameStarted{ false };

        std::unique_ptr<DescriptorPool> globalPool{};
        std::unique_ptr<SimpleRenderSystem> simpleRenderSystem;
        std::unique_ptr<PointLightSystem> pointLightSystem;
        std::unique_ptr<KeyboardMovementController> cameraController;

        std::vector<VkDescriptorSet> globalDescriptorSets;
        std::vector<std::unique_ptr<Buffer>> uboBuffers;
        std::vector<std::unique_ptr<Buffer>> bonesUboBuffers;
    };

}
