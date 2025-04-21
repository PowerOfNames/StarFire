#pragma once
#include <string>


#include "StarFire/Core/Core.h"

namespace StarFire {
	
	struct ApplicationSpecification
	{
		std::string Name;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specs);
		virtual ~Application();

		void Run();

		void Close();


		inline static Application* Get() { return s_Instance; }
		inline ApplicationSpecification& GetSpecification() { return m_Specification; }

	private:
		ApplicationSpecification m_Specification;


		static Application* s_Instance;
		bool m_Running;
	};

	Application* CreateApplication(int argc, char** argv);
}
