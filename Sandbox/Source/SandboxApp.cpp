#include "SandboxLayer.h"
#include "Profiling/Profiling.h"


#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>
#include <string_view>
#include <stdexcept>

namespace Sandbox {


	struct SandboxOptions
	{
		bool ThrowsInOnInit = false;
	};

	class SandboxApp : public StarFire::Application
	{
	public:
		SandboxApp(const StarFire::ApplicationSpecification& specs, const SandboxOptions& options)
			: Application(specs), m_Options(options)
		{
			PROFILE_FUNCTION;
		}

		void OnInit() override
		{
			if (m_Options.ThrowsInOnInit)
				throw std::runtime_error("Intentional test throw ");
			PushLayer(new SandboxLayer());
		}

		~SandboxApp()
		{
			PROFILE_FUNCTION;
		}
	private:
		SandboxOptions m_Options{};
	};

}

StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	PROFILE_FUNCTION;

	StarFire::ApplicationSpecification specs;
	specs.Name = "StarFire - Sandbox";
	specs.UseImGui = true;

	Sandbox::SandboxOptions options{};
	for (int i = 1; i < argc; i++)
	{
		std::string_view token = std::string_view(argv[i]);
		if (token == "--project" && i + 1 < argc)
		{
			specs.ProjectPath = argv[++i];
		}
		else if (token == "--width" && i+1 < argc)
		{
			if (auto width = StarFire::ApplicationArgumentParser::Parse<uint32_t>(argv[++i]))
				specs.WindowSpecs.Width = *width;			
		}
		else if (token == "--height" && i+1 < argc)
		{
			if (auto height = StarFire::ApplicationArgumentParser::Parse<uint32_t>(argv[++i]))
				specs.WindowSpecs.Height = *height;
		}
		else if (token == "--fullscreen")
		{
			specs.WindowSpecs.Fullscreen = true;
		}
		else if (token == "--frames" && i + 1 < argc)
		{
			if (auto maxFrames = StarFire::ApplicationArgumentParser::Parse<uint32_t>(argv[++i]))
				specs.MaxFrames = *maxFrames;
		}
		else if (token == "--throw-in-oninit")
		{
			options.ThrowsInOnInit = true;
		}
		else
			APP_WARN("Unknown token {} argument found", token);

	}

	return new Sandbox::SandboxApp(specs, options);
}

	

