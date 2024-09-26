#include <PCH/pch.h>
#include "Engine.h"

#include "Input/KeyboardController.h"
#include "Graphics/Vulkan/Buffers/Buffer.h"
#include "Graphics/Vulkan/Camera.h"
#include "Graphics/Vulkan/systems/PointLightSystem.h"
#include "Graphics/Vulkan/systems/SimpleRenderSystem.h"
#include "Graphics/Vulkan/Texture/Texture.h"
#include "Graphics/Vulkan/Texture/ImGuiTexture.h"

#include "glslang/Public/ShaderLang.h"

namespace ImGui {
    /**
     * Draw an ImGui::Image using Vulkan.
     *
     * \param mx: The texture manager to use.
     * \param index: The index of the texture to use. Index depends on the order in which images and models are loaded.
     * \param image_size: The size of the image.
     */
    void VulkanImage(TextureLibrary& mx, const size_t& index, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
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
    void VulkanImage(TextureLibrary& mx, const std::string& texturePath, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
    {
        auto ds = mx.GetDescriptorSet(texturePath);
        ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
    }
}

Engine::Engine()
    : textureLibrary(lveDevice, lveRenderer)
{
    globalPool =
        DescriptorPool::Builder(lveDevice)
        .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURE_COUNT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    textureLibrary.AddTexture("assets/textures/texture.jpg");
    textureLibrary.AddTexture("assets/textures/dog.png");
    textureLibrary.AddTexture("assets/textures/viking_room.png");
    textureLibrary.AddTexture("assets/models/textures/Book.png");

    loadGameObjects();

    glslang::InitializeProcess();
}

Engine::~Engine() {
    glslang::FinalizeProcess();
}

void Engine::initRayTracing()
{
    VkPhysicalDeviceProperties2 prop2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    prop2.pNext = &m_rtProperties;
    vkGetPhysicalDeviceProperties2(lveDevice.getPhysicalDevice(), &prop2);

    /* 31 bounces for my computer */
}

void Engine::run() {
    std::vector<std::unique_ptr<Buffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<Buffer>(
            lveDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY);
        uboBuffers[i]->map();
    }

    std::vector<std::unique_ptr<Buffer>> bonesUboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < bonesUboBuffers.size(); i++) {
        bonesUboBuffers[i] = std::make_unique<Buffer>(
            lveDevice,
            sizeof(BonesUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY);
        bonesUboBuffers[i]->map();
    }

    auto globalSetLayout =
        DescriptorSetLayout::Builder(lveDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_COUNT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    // Atleast 1 texture must be added by this point, or uh-oh.
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

    std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        auto boneBufferInfo = bonesUboBuffers[i]->descriptorInfo();

        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
            .writeBuffer(2, &boneBufferInfo)
            .build(globalDescriptorSets[i]);
    }

    /* Raytracin */
    initRayTracing();

    SimpleRenderSystem simpleRenderSystem{
        lveDevice,
        lveRenderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout(),
        textureLibrary
    };
    PointLightSystem pointLightSystem{
        lveDevice,
        lveRenderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout() };
    Camera camera{};

    auto viewerObject = GameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!lveWindow.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        // set window title to log fps
        static int updateFPS = 0;
        if (++updateFPS == 100) {
            lveWindow.setWindowTitle(("NiteLite Engine - " + std::to_string(1.f / frameTime) + " FPS").c_str());
            updateFPS = 0;
        }

        // animator->UpdateAnimation(frameTime);

        // log data from animator
        // printf("Current Animation Time: %f\n", animator->GetCurrentTime());

        cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = lveRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 8000.f);

        if (auto commandBuffer = lveRenderer.beginFrame()) {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();
            {
                // draw an imgui image
                ImGui::Begin("Texture");

                // loop through all textures and draw them
                ImGui::VulkanImage(textureLibrary, "assets/textures/texture.jpg", ImVec2(256, 256));

                /*for (size_t i = 1; i < textureLibrary.getTextureCount(); i++) {
                    ImGui::VulkanImage(textureLibrary, i, ImVec2(256, 256));
                }*/

                ImGui::End();
            }

            int frameIndex = lveRenderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects };

            // update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            /*BonesUbo bonesUbo{};
            auto transforms = animator->GetFinalBoneMatrices();
            for (size_t i = 0; i < transforms.size(); i++) {
                bonesUbo.finalBonesMatrices[i] = transforms[i];
            }
            bonesUboBuffers[frameIndex]->writeToBuffer(&bonesUbo);
            bonesUboBuffers[frameIndex]->flush();*/

            // render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(lveDevice.device());
}

void Engine::loadGameObjects() {
    std::shared_ptr<Model> lveModel = std::make_shared<Model>(lveDevice, "assets/models/quad.obj", textureLibrary);
    auto floor = GameObject::createGameObject();
    floor.model = lveModel;
    floor.transform.translation = { 0.f, 1.f, 0.f };
    floor.transform.scale = { 3.f, 1.f, 3.f };
    floor.textureIndex = 0;
    gameObjects.emplace(floor.getId(), std::move(floor));

    // gltf doesn't work properly rn
    /*lveModel =
        Model::createModelFromFile(lveDevice, "assets/models/scene.gltf", textureLibrary);
    auto sense = GameObject::createGameObject();
    sense.model = lveModel;
    sense.transform.translation = { 0.f, 0.f, 0.f };
    sense.transform.scale = { 1.f, 1.f, 1.f };
    gameObjects.emplace(sense.getId(), std::move(sense));*/

    lveModel = std::make_shared<Model>(lveDevice, "assets/models/charles.glb", textureLibrary);
    auto alyaModel = GameObject::createGameObject();
    alyaModel.model = lveModel;
    alyaModel.transform.translation = { 4.5f, 1.f, 0.f };
    alyaModel.transform.scale = { 1.f, -1.f, 1.f };
    //alyaModel.transform.rotation.x = 3.14f;
    alyaModel.transform.rotation.y = 3.14f;
    gameObjects.emplace(alyaModel.getId(), std::move(alyaModel));

    lveModel = std::make_shared<Model>(lveDevice, "assets/models/AlisaMikhailovna.fbx", textureLibrary);
    alyaModel = GameObject::createGameObject();
    alyaModel.model = lveModel;
    alyaModel.transform.translation = { -2.f, .5f, 0.4f };
    alyaModel.transform.scale = { 0.115f, -0.115f, 0.115f };
    //alyaModel.transform.rotation.x = 3.14f;
    //alyaModel.transform.rotation.y = 3.14f;
    gameObjects.emplace(alyaModel.getId(), std::move(alyaModel));

    lveModel = std::make_shared<Model>(lveDevice, "assets/models/Mon_BlackDragon31_Skeleton.FBX", textureLibrary);
    auto dragonKnight = GameObject::createGameObject();
    dragonKnight.model = lveModel;
    dragonKnight.transform.translation = { 0.f, 1.f, 0.f };
    //dragonKnight.transform.scale = { 0.1f, 0.1f, 0.1f };
    dragonKnight.transform.scale = { 0.1f, 0.1f, 0.1f };
    //dragonKnight.transform.rotation.y = 0.5f;
    dragonKnight.transform.rotation.x = -1.57f;
    gameObjects.emplace(dragonKnight.getId(), std::move(dragonKnight));

    /*animation = std::make_unique<Animation>("assets/models/Mon_BlackDragon31_Skeleton.FBX", lveModel.get());
    animator = std::make_unique<Animator>(animation.get());*/

    lveModel = std::make_shared<Model>(lveDevice, "assets/models/Book.fbx", textureLibrary);
    auto bookModel = GameObject::createGameObject();
    bookModel.model = lveModel;
    bookModel.transform.translation = { -1.5f, 0.f, 0.f };
    bookModel.transform.scale = { 2.f, 2.f, 2.f };
    bookModel.transform.rotation.x = 1.57f;
    bookModel.transform.rotation.y = 0.7f;
    bookModel.textureIndex = 3;
    gameObjects.emplace(bookModel.getId(), std::move(bookModel));

    lveModel = std::make_shared<Model>(lveDevice, "assets/models/smooth_vase.obj", textureLibrary);
    auto smoothVase = GameObject::createGameObject();
    smoothVase.model = lveModel;
    smoothVase.transform.translation = { .5f, .5f, 0.f };
    smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
    smoothVase.textureIndex = 1;
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

    lveModel = std::make_shared<Model>(lveDevice, "assets/models/viking_room.obj", textureLibrary);
    auto vikingRoom = GameObject::createGameObject();
    vikingRoom.model = lveModel;
    vikingRoom.transform.translation = { 0.f, .3f, 3.f };
    vikingRoom.transform.scale = { 2.f, 2.f, 2.f };
    vikingRoom.transform.rotation = glm::vec3(1.57f, 1.57f, 0.f);
    vikingRoom.textureIndex = 2;
    gameObjects.emplace(vikingRoom.getId(), std::move(vikingRoom));

    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}  //
    };

    for (size_t i = 0; i < lightColors.size(); i++) {
        auto pointLight = GameObject::makePointLight(0.2f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (i * glm::two_pi<float>()) / lightColors.size(),
            { 0.f, -1.f, 0.f });
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
}
