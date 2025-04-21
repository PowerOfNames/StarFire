#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>

#include <glm/glm.hpp>

class Sandbox : public StarFire::Application
{
public:
	Sandbox(const StarFire::ApplicationSpecification& specs)
		: Application(specs)
	{
		Run();
	}

	~Sandbox()
	{
	}
};

StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	StarFire::ApplicationSpecification specs;
	specs.Name = "StarFire - Sandbox";

	return new Sandbox(specs);
}



