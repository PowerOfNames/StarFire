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
		}

		void OnInit() override
		{
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
	for (int i = 1; i < argc; i++)
	{
		std::string_view token = std::string_view(argv[i]);
		if (token == "--project" && i + 1 < argc)
		{
			specs.ProjectPath = argv[++i];
		}
	}

	return new Nebula::NebulaApp(specs);
}