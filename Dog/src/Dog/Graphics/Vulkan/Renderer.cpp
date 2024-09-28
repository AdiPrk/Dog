#include <PCH/pch.h>
#include "Renderer.h"
#include "Systems/SimpleRenderSystem.h"
#include "Systems/PointLightSystem.h"
#include "Camera.h"
#include "Descriptors/Descriptors.h"
#include "Texture/TextureLibrary.h"
#include "Models/ModelLibrary.h"
#include "glslang/Public/ShaderLang.h"
#include "Core/SwapChain.h"
#include "Input/KeyboardController.h"
#include "Entities/GameObject.h"
#include "Input/input.h"

#include "Engine.h"

#include "Graphics/Editor/Editor.h"

namespace Dog {

    Renderer::Renderer(Window& window, Device& device)
        : m_Window{ window }
        , device{ device }
        , globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        , uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT)
        , bonesUboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT)
    {
        recreateSwapChain();
        createCommandBuffers();

        globalPool =
            DescriptorPool::Builder(device)
            .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURE_COUNT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

        glslang::InitializeProcess();

        Input::Init(m_Window.getGLFWwindow());
    }

    Renderer::~Renderer() {
        freeCommandBuffers();
        glslang::FinalizeProcess();
    }

    void Renderer::Init()
    {
        auto& textureLibrary = Engine::Get().GetTextureLibrary();
        auto& modelLibrary = Engine::Get().GetModelLibrary();

        textureLibrary.AddTexture("assets/textures/square.png");
        textureLibrary.AddTexture("assets/textures/texture.jpg");
        textureLibrary.AddTexture("assets/textures/dog.png");
        textureLibrary.AddTexture("assets/textures/viking_room.png");
        textureLibrary.AddTexture("assets/models/ModelTextures/Book.png");

        modelLibrary.AddModel("assets/models/quad.obj");
        modelLibrary.AddModel("assets/models/charles.glb");
        modelLibrary.AddModel("assets/models/AlisaMikhailovna.fbx");
        modelLibrary.AddModel("assets/models/Mon_BlackDragon31_Skeleton.FBX");
        modelLibrary.AddModel("assets/models/Book.fbx");
        modelLibrary.AddModel("assets/models/smooth_vase.obj");
        modelLibrary.AddModel("assets/models/viking_room.obj");

        for (size_t i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(
                device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY);
            uboBuffers[i]->map();
        }

        for (size_t i = 0; i < bonesUboBuffers.size(); i++) {
            bonesUboBuffers[i] = std::make_unique<Buffer>(
                device,
                sizeof(BonesUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY);
            bonesUboBuffers[i]->map();
        }

        auto globalSetLayout =
            DescriptorSetLayout::Builder(device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_COUNT)
            .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        // Atleast 1 texture must be added by this point, or uh-oh.
        // Set up image infos for the descriptor set
        std::vector<VkDescriptorImageInfo> imageInfos(MAX_TEXTURE_COUNT);
        for (size_t j = 0; j < MAX_TEXTURE_COUNT; j++) {
            if (j < textureLibrary.getTextureCount()) {
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[j].imageView = textureLibrary.getTextureByIndex(j).getImageView();
                imageInfos[j].sampler = textureLibrary.getTextureByIndex(j).getSampler();
            }
            else {
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[j].imageView = textureLibrary.getTextureByIndex(0).getImageView();
                imageInfos[j].sampler = textureLibrary.getTextureByIndex(0).getSampler();
            }
        }

        // Create descriptor sets
        for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto boneBufferInfo = bonesUboBuffers[i]->descriptorInfo();

            DescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
                .writeBuffer(2, &boneBufferInfo)
                .build(globalDescriptorSets[i]);
        }

        simpleRenderSystem = std::make_unique<SimpleRenderSystem>(
			device,
			getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout(),
			textureLibrary,
			modelLibrary);

        pointLightSystem = std::make_unique<PointLightSystem>(
            device,
            getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout());       

        // Temporary camera controller
        cameraController = std::make_unique<KeyboardMovementController>();
    }

    void Renderer::Render(float dt, GameObject::Map& gameObjects)
    {
        auto& textureLibrary = Engine::Get().GetTextureLibrary();
        auto& modelLibrary = Engine::Get().GetModelLibrary();

        // should become a component of the viewer object
        static Camera camera{};

        // Make a viewer object which will hold the camera
        // should become an entity once that exists
        static GameObject viewerObject = GameObject::createGameObject();
        // viewerObject.transform.translation.z = -2.5f;


        cameraController->moveInPlaneXZ(m_Window.getGLFWwindow(), dt, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        // Set the camera's projection
        float aspect = getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 8000.f);

        // Start the frame
        if (auto commandBuffer = beginFrame()) {
            Engine::Get().GetEditor().BeginFrame();

            int frameIndex = getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                dt,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects };

            // update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem->update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // animator->UpdateAnimation(frameTime);
            /*BonesUbo bonesUbo{};
            auto transforms = animator->GetFinalBoneMatrices();
            for (size_t i = 0; i < transforms.size(); i++) {
                bonesUbo.finalBonesMatrices[i] = transforms[i];
            }
            bonesUboBuffers[frameIndex]->writeToBuffer(&bonesUbo);
            bonesUboBuffers[frameIndex]->flush();*/

            // render
            beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem->renderGameObjects(frameInfo);
            pointLightSystem->render(frameInfo);

            Engine::Get().GetEditor().EndFrame(commandBuffer);

            endSwapChainRenderPass(commandBuffer);
            endFrame();
        }
    }

    void Renderer::Exit()
    {
        // Wait until the device is idle before cleaning up resources
		vkDeviceWaitIdle(device);
    }

    void Renderer::recreateSwapChain() {
        auto extent = m_Window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_Window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device);

        if (m_SwapChain == nullptr) {
            m_SwapChain = std::make_unique<SwapChain>(device, extent);
        }
        else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*m_SwapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            device,
            device.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame() {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        auto result = m_SwapChain->acquireNextImage(&currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return commandBuffer;
    }

    void Renderer::endFrame() {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");

        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_SwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_Window.wasWindowResized()) {
            m_Window.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_SwapChain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }

} // namespace Dog
