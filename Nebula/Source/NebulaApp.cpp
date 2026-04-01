#include "EditorLayer.h"
#include "Profiling/Profiling.h"

#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>

namespace Nebula {

	class NebulaApp : public StarFire::Application
	{
	public:
		NebulaApp(const StarFire::ApplicationSpecification& specs)
			: Application(specs)
		{
			PROFILE_FUNCTION;

			PushLayer(new EditorLayer());
		}

		~NebulaApp()
		{
			PROFILE_FUNCTION;

		}
	};
}
StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	PROFILE_FUNCTION;

	StarFire::ApplicationSpecification specs;
	specs.Name = "Nebula";

	return new Nebula::NebulaApp(specs);
}