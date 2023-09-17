#pragma once

#include "Core.h"

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