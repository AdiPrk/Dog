#pragma once

#include "Graphics/Vulkan/Descriptors/Descriptors.h"
#include "Graphics/Vulkan/Core/Device.h"
#include "Entities/GameObject.h"
#include "Graphics/Vulkan/Renderer.h"
#include "Graphics/Vulkan/Window/Window.h"
#include "Graphics/Vulkan/Texture/TextureLibrary.h"
#include "Graphics/Vulkan/Models/Animation.h"
#include "Graphics/Vulkan/Models/Animator.h"

// std
#include <memory>
#include <vector>

class Engine {
public:
	static constexpr int WIDTH = 1600;
	static constexpr int HEIGHT = 900;

	Engine();
	~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// RAYTRACIN STUFF
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
	void initRayTracing();

	void run();

private:
	void loadGameObjects();

	LveWindow lveWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
	LveDevice lveDevice{ lveWindow };
	LveRenderer lveRenderer{ lveWindow, lveDevice };

	// note: order of declarations matters
	std::unique_ptr<DescriptorPool> globalPool{};
	GameObject::Map gameObjects;

	TextureLibrary textureLibrary;

	// Animation
	std::unique_ptr<Animation> animation;
	std::unique_ptr<Animator> animator;
};