#pragma once

class LveWindow {
public:
	LveWindow(int w, int h, std::string name);
	~LveWindow();

	LveWindow(const LveWindow&) = delete;
	LveWindow& operator=(const LveWindow&) = delete;

	bool shouldClose() { return glfwWindowShouldClose(window); }
	VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
	bool wasWindowResized() { return framebufferResized; }
	void resetWindowResizedFlag() { framebufferResized = false; }
	GLFWwindow* getGLFWwindow() const { return window; }
	void setWindowTitle(const char* title) { glfwSetWindowTitle(window, title); }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void initWindow();

	int width;
	int height;
	bool framebufferResized = false;

	std::string windowName;
	GLFWwindow* window;
};