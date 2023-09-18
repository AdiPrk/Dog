#include "dogpch.h"

#include "Application.h"

#include "Events/ApplicationEvents.h"

namespace Dog {

	Application::Application() {

	}

	Application::~Application() {

	}

	void Application::Run() {
		WindowResizeEvent e(1280, 720);
		DOG_TRACE(e);

		while (true) {};
	}
}