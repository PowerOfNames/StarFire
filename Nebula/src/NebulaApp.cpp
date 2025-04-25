#include <StarFire.h>
#include <StarFire/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Nebula {

	class NebulaApp : public StarFire::Application
	{
	public:
		NebulaApp(const StarFire::ApplicationSpecification& specs)
			: Application(specs)
		{
			PushLayer(new EditorLayer());
		}

		~NebulaApp()
		{
		}
	};
}
StarFire::Application* StarFire::CreateApplication(int argc, char** argv)
{
	StarFire::ApplicationSpecification specs;
	specs.Name = "Nebula";

	return new Nebula::NebulaApp(specs);
}