#include "dogpch.h"

#include "Application.h"

#include "Events/ApplicationEvents.h"

#include "GLFW/glfw3.h"

namespace Dog {

	Application::Application() {

	}

	Application::~Application() {

	}

	void Application::Run() {
		int i = glfwInit();
		DOG_CORE_WARN("Glfw init: {0}", i);

		WindowResizeEvent e(1280, 720);
		DOG_TRACE(e);

		while (true) {};
	}
}