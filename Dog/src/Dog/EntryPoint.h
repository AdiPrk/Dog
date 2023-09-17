#pragma once

#ifdef DOG_PLATFORM_WINDOWS

extern Dog::Application* Dog::CreateApplication();

int main(int argc, char** argv) {
	auto app = Dog::CreateApplication();
	app->Run();
	delete app;
}

#endif