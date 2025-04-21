#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>

class NebulaApp : public StarFire::Application
{
public:
	NebulaApp(const StarFire::ApplicationSpecification& specs)
		: Application(specs)
	{
		Run();
	}

	~NebulaApp()
	{
	}
};

StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	StarFire::ApplicationSpecification specs;
	specs.Name = "Nebula";

	return new NebulaApp(specs);
}
