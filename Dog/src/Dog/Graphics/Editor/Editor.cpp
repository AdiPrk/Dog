#include <PCH/pch.h>

#ifndef DOG_SHIP

#include "editor.h"
#include "Dog/engine.h"
#include "Dog/Input/input.h"

// #include "Windows/sceneWindow.h"
// #include "Windows/consoleWindow.h"
#include "Windows/EntitiesWindow.h"
#include "Windows/InspectorWindow.h"
// #include "Windows/toolbarWindow.h"
#include "Windows/assetsWindow.h"
// #include "Windows/textEditorWindow.h"
// #include "Windows/noEditorWindow.h"

#include "Dog/Scene/sceneManager.h"
#include "Dog/Scene/scene.h"
#include "Dog/Graphics/Vulkan/Window/Window.h"
#include "Dog/Graphics/Vulkan/Core/Device.h"
#include "Dog/Graphics/Vulkan/Core/SwapChain.h"

#include "Scene/Serializer/SceneSerializer.h"

namespace Dog {

	/*
	
	namespace ImGui {
    /**
     * Draw an ImGui::Image using Vulkan.
     *
     * \param mx: The texture manager to use.
     * \param index: The index of the texture to use. Index depends on the order in which images and models are loaded.
     * \param image_size: The size of the image.
     */
	/*void VulkanImage(Dog::TextureLibrary& mx, const size_t& index, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
	{
		auto ds = mx.GetDescriptorSetByIndex(index);
		ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
	}
	*/

	/**
	 * Draw an ImGui::Image using Vulkan.
	 *
	 * \param mx: The texture manager to use.
	 * \param texturePath: The path to the texture to use.
	 * \param image_size: The size of the image.
	 */
	/*
	void VulkanImage(Dog::TextureLibrary& mx, const std::string& texturePath, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
	{
		auto ds = mx.GetDescriptorSet(texturePath);
		ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
	}
	*/

	Editor::Editor()
	{
		fileBrowser = std::make_unique<FileBrowser>();
		// textEditorWrapper = std::make_unique<TextEditorWrapper>();
	}

	Editor::~Editor()
	{
	}

	void Editor::Init()
	{
		Device& device = Engine::Get().GetDevice();
		Window& window = Engine::Get().GetWindow();
		SwapChain& swapChain = Engine::Get().GetRenderer().GetSwapChain();

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
		ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.getInstance();
		init_info.PhysicalDevice = device.getPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = device.GetGraphicsFamily();
		init_info.Queue = device.graphicsQueue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = imGuiDescriptorPool;// device.getImGuiDescriptorPool();
		init_info.RenderPass = swapChain.getRenderPass();
		init_info.Subpass = 0;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = static_cast<uint32_t>(swapChain.imageCount());
		init_info.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&init_info);

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDark();
	}

	void Editor::Exit()
	{
		Device& device = Engine::Get().GetDevice();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorSetLayout(device, samplerSetLayout, nullptr);
		vkDestroyDescriptorPool(device, imGuiDescriptorPool, nullptr);
	}

	void Editor::BeginFrame()
	{
		if (!isActive) {
			return;
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene")) {}
			if (ImGui::MenuItem("Save Scene"))
			{
				Scene* currentScene = SceneManager::GetCurrentScene();
				if (currentScene != nullptr) {
					std::string sceneName = currentScene->GetName();

					SceneSerializer::Serialize(currentScene, "assets/scenes/" + sceneName + ".yaml");
				}
				else {
					DOG_WARN("No scene to save!");
				}
			}

			// open scene, which shows all scenes
			if (ImGui::BeginMenu("Open Scene")) {

				// get all files in the scene directory (std filesystem)
				std::vector<std::string> files;
				for (const auto& entry : std::filesystem::directory_iterator("assets/scenes")) {
					files.push_back(entry.path().string());
				}

				// display all files in the scene directory
				for (const auto& file : files) {

					// remove the directory path
					std::string fileName = file.substr(file.find_last_of("/\\") + 1);

					if (ImGui::MenuItem(fileName.c_str())) {

						Scene* currentScene = SceneManager::GetCurrentScene();

						if (currentScene != nullptr) {
							std::string sceneName = currentScene->GetName();

							SceneSerializer::Deserialize(currentScene, "assets/scenes/" + sceneName + ".yaml");
						}
						else {
							DOG_WARN("No scene to deserialize to????");
						}

					}
				}

				ImGui::EndMenu(); // Open Scene
			}


			if (ImGui::MenuItem("Exit")) {
				Engine::Get().Exit();
			}
			if (ImGui::MenuItem("Create Asset Pack")) {
				// DogFilePacker::packageAssets("DogAssets", "fetch");
			}
			ImGui::EndMenu();
		}

		// FPS
		char fpsText[32];
		sprintf_s(fpsText, "FPS: %.1f", ImGui::GetIO().Framerate);

		ImVec2 textSize = ImGui::CalcTextSize(fpsText, NULL, true);
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textSize.x - 10);
		ImGui::Text(fpsText);

		ImGui::EndMainMenuBar(); // File

		ImGui::ShowDemoWindow();

		// Update and render all the different windows
		// UpdateSceneWindow(doResize);
		// UpdateConsoleWindow();
		UpdateEntitiesWindow();
		UpdateInspectorWindow();
		// UpdateToolbarWindow();
		UpdateAssetsWindow(*fileBrowser);
		// UpdateTextEditorWindow(*textEditorWrapper);
	}

	void Editor::EndFrame(VkCommandBuffer commandBuffer)
	{
		if (!isActive) {
			return;
		}

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}

	void Editor::UpdateVisibility(unsigned windowWidth, unsigned windowHeight)
	{
		ImGuiIO& io = ImGui::GetIO();

		static bool renderEditor = isActive;
		static bool keyHeld = false;
		bool firstGameFrame = false;
		if (io.KeyCtrl && io.KeyShift && io.KeysDown[ImGuiKey_J])
		{
			if (!keyHeld) {
				renderEditor = !renderEditor;
				isActive = renderEditor;
				keyHeld = true;
				firstGameFrame = true;

				if (!renderEditor) {
					PUBLISH_EVENT(Event::SceneResize, (int)windowWidth, (int)windowHeight);
					glViewport(0, 0, windowWidth, windowHeight);
					Input::SetKeyInputLocked(false);
					Input::SetMouseInputLocked(false);
				}
			}
		}
		else {
			keyHeld = false;
		}
	}

} // namespace Dog

#endif // DOG_SHIP
