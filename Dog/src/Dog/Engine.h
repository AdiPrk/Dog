#pragma once

#include "Graphics/Vulkan/Descriptors/Descriptors.h"
#include "Graphics/Vulkan/Core/Device.h"
#include "Entities/GameObject.h"
#include "Graphics/Vulkan/Renderer.h"
#include "Graphics/Vulkan/Window/Window.h"
#include "Graphics/Vulkan/Texture/TextureLibrary.h"
#include "Graphics/Vulkan/Models/ModelLibrary.h"
#include "Graphics/Vulkan/Animation/Animation.h"
#include "Graphics/Vulkan/Animation/Animator.h"

namespace Dog {

	struct EngineSpec {
		std::string name = "Dog Engine"; // The name of the window.
		unsigned width = 1280;           // The width of the window.
		unsigned height = 720;           // The height of the window.
		unsigned fps = 60;			     // The target frames per second.
	};

	class Engine {
	public:
		static constexpr int WIDTH = 1600;
		static constexpr int HEIGHT = 900;

		Engine(const EngineSpec& specs);
		~Engine();

		// Called by client to create the engine.
		static Engine& Create(const EngineSpec& specs = {})
		{
			static Engine instance(specs);
			return instance;
		}

		static Engine& Get()
		{
			return Create();
		}

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		void Run();

		// getters
		Window& GetWindow() { return m_Window; }
		Device& GetDevice() { return device; }
		Renderer& GetRenderer() { return *m_Renderer; }
		TextureLibrary& GetTextureLibrary() { return textureLibrary; }
		ModelLibrary& GetModelLibrary() { return modelLibrary; }

	private:
		void loadGameObjects();

		Window m_Window; // { WIDTH, HEIGHT, "Woof" };
		Device device{ m_Window };
		std::unique_ptr<Renderer> m_Renderer;

		// note: order of declarations matters
		GameObject::Map gameObjects;

		TextureLibrary textureLibrary;
		ModelLibrary modelLibrary;

		// Animation
		std::unique_ptr<Animation> animation;
		std::unique_ptr<Animator> animator;

		// target fps
		unsigned fps;
	};

} // namespace Dog
