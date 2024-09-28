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

#include "Graphics/Editor/Editor.h"

namespace Dog {

    Engine::Engine(const EngineSpec& specs)
        : m_Window(specs.width, specs.height, specs.name)
        , m_Renderer(std::make_unique<Renderer>(m_Window, device))
        , textureLibrary(device)
        , modelLibrary(device, textureLibrary)
        , fps(specs.fps)
    {
        Logger::Init();
        m_Editor = std::make_unique<Editor>();
    }

    Engine::~Engine() {
        m_Editor->Exit();
    }

    void Engine::Run(const std::string& sceneName) {
        // Init some stuff
        m_Editor->Init();
        m_Renderer->Init();

        SceneManager::Init(sceneName);
        //SceneManager::SwapScenes();

        //Scene& scene = *SceneManager::GetCurrentScene();

        //Entity quad = scene.CreateEntity(); // default model
        //quad.GetComponent<TransformComponent>().Translation = { 0.f, 0.1f, 0.f };

        //Entity alisa = scene.CreateEntity("Hai");
        //alisa.GetComponent<ModelComponent>().SetModel("assets/models/AlisaMikhailovna.fbx");
        //alisa.GetComponent<TransformComponent>().Translation = { 0.f, 0.1f, 0.5f };
        //alisa.GetComponent<TransformComponent>().Rotation = { 1.57f, 0.f, 0.f };
        //alisa.GetComponent<TransformComponent>().Scale = glm::vec3(4.f);

        //Entity charles = scene.CreateEntity("Charles");
        //charles.GetComponent<ModelComponent>().SetModel("assets/models/charles.glb");
        //charles.GetComponent<TransformComponent>().Translation = { 2.5f, 0.1f, 0.5f };
        //charles.GetComponent<TransformComponent>().Scale = { -1.f, -1.f, -1.f };

        //FrameRateController frameRateController(fps);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!m_Window.shouldClose() && m_Running) {
            Input::Update();
            
            // Need to move in frame rate controller
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // Swap scenes if necessary (also does Init/Exit)
            SceneManager::SwapScenes();

            // Update scenes
            SceneManager::Update(frameTime);

            // Render scenes (doesn't do anything rn)
            SceneManager::Render(frameTime, false);

            m_Renderer->Render(frameTime, gameObjects); // actual render
        }

        m_Renderer->Exit();
    }
    void Engine::Exit()
    {
        m_Running = false;
    }

} // namespace Dog