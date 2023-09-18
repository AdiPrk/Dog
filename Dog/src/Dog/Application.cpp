#include "dogpch.h"

#include "Application.h"

#include "Events/ApplicationEvents.h"

#include "GLFW/glfw3.h"

namespace Dog {

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() {

	}

	void Application::Run() {
		// event test
		WindowResizeEvent e(1280, 720);
		DOG_TRACE(e);

		while (m_Running)
		{
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}
}