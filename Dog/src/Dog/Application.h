#pragma once

#include "Core.h"
#include "Events/Events.h"
#include "Window.h"

namespace Dog {

	class DOG_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private: 
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// to be defined in client
	Application* CreateApplication();
}