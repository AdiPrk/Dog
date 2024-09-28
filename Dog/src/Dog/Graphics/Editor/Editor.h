#pragma once

#ifndef DOG_SHIP

namespace Dog {

	class FileBrowser;
	class TextEditorWrapper;

	class Editor {
	public:
		Editor();
		~Editor();

		static Editor& Get() {
			static Editor instance;
			return instance;
		}

		Editor(const Editor&) = delete;
		Editor& operator=(const Editor&) = delete;

		void Init();
		void Exit();

		void BeginFrame();
		void EndFrame(VkCommandBuffer commandBuffer);

		void SetEditorEnabled(bool enabled) { renderEditor = enabled; }
		bool GetEditorEnabled() const { return renderEditor; }

		void UpdateVisibility(unsigned windowWidth, unsigned windowHeight);
		bool IsActive() const { return isActive; }

		void DoSceneResize() { doResize = true; }

		VkDescriptorPool imGuiDescriptorPool;
		VkDescriptorSetLayout samplerSetLayout;
	private:
		bool isActive = true;
		bool doResize = false;

		std::unique_ptr<FileBrowser> fileBrowser;
		//std::unique_ptr<TextEditorWrapper> textEditorWrapper;
		bool renderEditor = true;
		bool startedRenderingFrame = false;

	};

} // namespace Dog

#endif