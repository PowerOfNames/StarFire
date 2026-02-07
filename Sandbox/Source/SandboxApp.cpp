#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>

#include "SandboxLayer.h"

namespace Sandbox {

	class SandboxApp : public StarFire::Application
	{
	public:
		SandboxApp(const StarFire::ApplicationSpecification& specs)
			: Application(specs)
		{
			PushLayer(new SandboxLayer());
		}

		~SandboxApp()
		{
		}
	};
}

StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	StarFire::ApplicationSpecification specs;
	specs.Name = "StarFire - Sandbox";

	return new Sandbox::SandboxApp(specs);
}

	

