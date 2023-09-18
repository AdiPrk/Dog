#pragma once

#include "Core.h"
#include "Events/Events.h"

namespace Dog {

	class DOG_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// to be defined in client
	Application* CreateApplication();
}