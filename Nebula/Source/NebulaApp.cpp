#include "NebulaMainLayer.h"
#include "Profiling/Profiling.h"

#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>

namespace Nebula {

	struct NebulaLayerSpecification
	{
		uint32_t ViewportWidth = 1280;
		uint32_t ViewportHeight = 720;
	};

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
			NebulaLayerSpecification layerSpecs{};
			layerSpecs.ViewportWidth = m_Specification.WindowSpecs.Width;
			layerSpecs.ViewportHeight = m_Specification.WindowSpecs.Height;
			PushLayer(new NebulaMainLayer());
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
	// Should always be enabled, regardless of command line args.
	// The editor layer will handle the ImGui logic and whether it is active or not
	specs.EnableImGui = true;

	for (int i = 1; i < argc; i++)
	{
		std::string_view token = std::string_view(argv[i]);
		if (token == "--project" && i + 1 < argc)
		{
			specs.ProjectPath = argv[++i];
		}
		else if (token == "--width" && i + 1 < argc)
		{
			if (auto width = StarFire::ApplicationArgumentParser::Parse<uint32_t>(argv[++i]))
				specs.WindowSpecs.Width = *width;
		}
		else if (token == "--height" && i + 1 < argc)
		{
			if (auto height = StarFire::ApplicationArgumentParser::Parse<uint32_t>(argv[++i]))
				specs.WindowSpecs.Height = *height;
		}
		else if (token == "--fullscreen")
		{
			specs.WindowSpecs.Fullscreen = true;
		}
		else if (token == "--imGui")
		{			
			APP_WARN("ImGui is mandatory fo Nebula and cant be deactivated. Omitting argument", token);
		}
		else
			APP_WARN("Unknown token {} argument found", token);
	}

	return new Nebula::NebulaApp(specs);
}