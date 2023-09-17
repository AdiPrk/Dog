#pragma once

#ifdef DOG_PLATFORM_WINDOWS

extern Dog::Application* Dog::CreateApplication();

int main(int argc, char** argv) {
	Dog::Log::Init();
	DOG_CORE_WARN("Initialized Logger!");
	DOG_INFO("Woof! {0}", 5);

	auto app = Dog::CreateApplication();
	app->Run();
	delete app;
}

#endif