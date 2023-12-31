#pragma once

#include "Core.h"
#include "Events/Events.h"
#include "Window.h"
#include "Events/ApplicationEvents.h"
#include "LayerStack.h"

namespace Dog {

	class DOG_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	private: 
		bool OnWindowClosed(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_Instance; // singleton
	};

	// to be defined in client
	Application* CreateApplication();
}