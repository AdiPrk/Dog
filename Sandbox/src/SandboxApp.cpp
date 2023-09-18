#include <Dog.h>

class ExampleLayer : public Dog::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		DOG_INFO("ExampleLayer::Update");
	}

	void OnEvent(Dog::Event& event) override
	{
		DOG_TRACE("{0}", event);
	}

};

class Sandbox : public Dog::Application 
{
public:
	Sandbox() {
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {

	}

};

Dog::Application* Dog::CreateApplication() {
	return new Sandbox();
}