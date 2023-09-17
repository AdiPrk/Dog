#include <Dog.h>

class Sandbox : public Dog::Application 
{
public:
	Sandbox() {

	}
	~Sandbox() {

	}

};

Dog::Application* Dog::CreateApplication() {
	return new Sandbox();
}