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

namespace ImGui {
    /**
     * Draw an ImGui::Image using Vulkan.
     *
     * \param mx: The texture manager to use.
     * \param index: The index of the texture to use. Index depends on the order in which images and models are loaded.
     * \param image_size: The size of the image.
     */
    void VulkanImage(Dog::TextureLibrary& mx, const size_t& index, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
    {
        auto ds = mx.GetDescriptorSetByIndex(index);
        ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
    }

    /**
     * Draw an ImGui::Image using Vulkan.
     *
     * \param mx: The texture manager to use.
     * \param texturePath: The path to the texture to use.
     * \param image_size: The size of the image.
     */
    void VulkanImage(Dog::TextureLibrary& mx, const std::string& texturePath, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
    {
        auto ds = mx.GetDescriptorSet(texturePath);
        ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
    }
}


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

        VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = MAX_TEXTURE_COUNT;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        vkCreateDescriptorPool(device, &pool_info, VK_NULL_HANDLE, &imGuiDescriptorPool);

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;  // Use your own sampler

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &samplerLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &samplerSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        // init imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(m_Window.getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device;
        init_info.QueueFamily = device.GetGraphicsFamily();
        init_info.Queue = device.graphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = imGuiDescriptorPool;// device.getImGuiDescriptorPool();
        init_info.RenderPass = lveSwapChain->getRenderPass();
        init_info.Subpass = 0;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount = static_cast<uint32_t>(lveSwapChain->imageCount());
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info);

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
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorSetLayout(device, samplerSetLayout, nullptr);
        vkDestroyDescriptorPool(device, imGuiDescriptorPool, nullptr);

        freeCommandBuffers();
    }

    void Renderer::Init()
    {
        auto& textureLibrary = Engine::Get().GetTextureLibrary();
        auto& modelLibrary = Engine::Get().GetModelLibrary();

        textureLibrary.AddTexture("assets/textures/square.png");
        textureLibrary.AddTexture("assets/textures/texture.jpg");
        textureLibrary.AddTexture("assets/textures/dog.png");
        textureLibrary.AddTexture("assets/textures/viking_room.png");
        textureLibrary.AddTexture("assets/models/textures/Book.png");

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
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // FPS
            ImGui::BeginMainMenuBar();

            char fpsText[32];
            sprintf_s(fpsText, "FPS: %.1f", ImGui::GetIO().Framerate);

            ImVec2 textSize = ImGui::CalcTextSize(fpsText, NULL, true);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textSize.x - 10);
            ImGui::Text(fpsText);

            ImGui::EndMainMenuBar();

            ImGui::ShowDemoWindow();
            {
                // draw an imgui image
                ImGui::Begin("Texture");

                // loop through all textures and draw them
                ImGui::VulkanImage(textureLibrary, "assets/textures/texture.jpg", ImVec2(256, 256));

                for (size_t i = 1; i < textureLibrary.getTextureCount(); i++) {
                    ImGui::VulkanImage(textureLibrary, i, ImVec2(256, 256));
                }

                ImGui::End();
            }

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

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

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

        if (lveSwapChain == nullptr) {
            lveSwapChain = std::make_unique<SwapChain>(device, extent);
        }
        else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(lveSwapChain);
            lveSwapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
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

        auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
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

        auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
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
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, lveSwapChain->getSwapChainExtent() };
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
