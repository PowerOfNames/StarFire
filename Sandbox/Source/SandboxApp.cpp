#include "SandboxLayer.h"
#include "Profiling/Profiling.h"


#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>


namespace Sandbox {

	class SandboxApp : public StarFire::Application
	{
	public:
		SandboxApp(const StarFire::ApplicationSpecification& specs)
			: Application(specs)
		{
			PROFILE_FUNCTION;

			PushLayer(new SandboxLayer());
		}

		~SandboxApp()
		{
			PROFILE_FUNCTION;

		}
	};
}

StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	PROFILE_FUNCTION;

	StarFire::ApplicationSpecification specs;
	specs.Name = "StarFire - Sandbox";
	specs.UseImGui = true;

	return new Sandbox::SandboxApp(specs);
}

	

