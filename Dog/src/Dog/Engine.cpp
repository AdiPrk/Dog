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

#include "Input/input.h"

#include "Graphics/Vulkan/Window/FrameRate.h"

#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Entity/Components.h"

namespace Dog {

    Engine::Engine(const EngineSpec& specs)
        : m_Window(specs.width, specs.height, specs.name)
        , m_Renderer(std::make_unique<Renderer>(m_Window, device))
        , textureLibrary(device, *m_Renderer)
        , modelLibrary(device, textureLibrary)
        , fps(specs.fps)
    {
        Logger::Init();
        // make frame rate controller with specs.fps
    }

    Engine::~Engine() {
        glslang::FinalizeProcess();
    }

    void Engine::Run() {
        // Init some stuff
        m_Renderer->Init();
        loadGameObjects();

        SceneManager::Init("Test");
        SceneManager::SwapScenes();

        Scene& scene = *SceneManager::GetCurrentScene();

        Entity ent = scene.CreateEntity("Hai");

        auto& tag = ent.GetComponent<TagComponent>();
        auto& tr = ent.GetComponent<TransformComponent>();

        // log them
        DOG_INFO("Tag: {0}", tag.Tag);
        DOG_INFO("Transform: {0}, {1}, {2}", tr.Translation.x, tr.Translation.y, tr.Translation.z);

        //FrameRateController frameRateController(fps);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!m_Window.shouldClose()) {
            Input::Update();
            
            // Need to move in frame rate controller
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            m_Renderer->Render(frameTime, gameObjects);
        }

        m_Renderer->Exit();
    }

    void Engine::loadGameObjects() {
        std::shared_ptr<Model> lveModel = std::make_shared<Model>(device, "assets/models/quad.obj", textureLibrary);
        auto floor = GameObject::createGameObject();
        floor.model = modelLibrary.GetModel("assets/models/quad.obj");
        floor.transform.translation = { 0.f, 1.f, 0.f };
        floor.transform.scale = { 3.f, 1.f, 3.f };
        floor.textureIndex = 0;
        gameObjects.emplace(floor.getId(), std::move(floor));

        auto alyaModel = GameObject::createGameObject();
        alyaModel.model = modelLibrary.GetModel("assets/models/charles.glb");
        alyaModel.transform.translation = { 4.5f, 1.f, 0.f };
        alyaModel.transform.scale = { 1.f, -1.f, 1.f };
        alyaModel.transform.rotation.y = 3.14f;
        gameObjects.emplace(alyaModel.getId(), std::move(alyaModel));

        alyaModel = GameObject::createGameObject();
        alyaModel.model = modelLibrary.GetModel("assets/models/AlisaMikhailovna.fbx");
        alyaModel.transform.translation = { -2.f, .5f, 0.4f };
        alyaModel.transform.scale = { 0.115f, -0.115f, 0.115f };
        gameObjects.emplace(alyaModel.getId(), std::move(alyaModel));

        auto dragonKnight = GameObject::createGameObject();
        dragonKnight.model = modelLibrary.GetModel("assets/models/Mon_BlackDragon31_Skeleton.FBX");
        dragonKnight.transform.translation = { -1.f, 1.f, 0.f };
        dragonKnight.transform.scale = { 0.1f, -0.1f, -0.1f };
        dragonKnight.transform.rotation.x = -1.57f;
        gameObjects.emplace(dragonKnight.getId(), std::move(dragonKnight));

        /*animation = std::make_unique<Animation>("assets/models/Mon_BlackDragon31_Skeleton.FBX", lveModel.get());
        animator = std::make_unique<Animator>(animation.get());*/

        auto bookModel = GameObject::createGameObject();
        bookModel.model = modelLibrary.GetModel("assets/models/Book.fbx");
        bookModel.transform.translation = { -1.5f, 0.f, 0.f };
        bookModel.transform.scale = { 2.f, 2.f, 2.f };
        bookModel.transform.rotation.x = 1.57f;
        bookModel.transform.rotation.y = 0.7f;
        bookModel.textureIndex = 3;
        gameObjects.emplace(bookModel.getId(), std::move(bookModel));

        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = modelLibrary.GetModel("assets/models/smooth_vase.obj");
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        smoothVase.textureIndex = 1;
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        auto vikingRoom = GameObject::createGameObject();
        vikingRoom.model = modelLibrary.GetModel("assets/models/viking_room.obj");
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

} // namespace Dog