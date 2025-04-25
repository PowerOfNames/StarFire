#pragma once
#include <string>

#include "StarFire/Core/Core.h"
#include "StarFire/Core/LayerStack.h"


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


		void PushOverlay(Layer* overlay);
		void PushLayer(Layer* layer);

		void Close();


		inline static Application* Get() { return s_Instance; }
		inline ApplicationSpecification& GetSpecification() { return m_Specification; }

	private:
		ApplicationSpecification m_Specification;
		static Application* s_Instance;

		bool m_Running;

		LayerStack m_LayerStack;
	};

	Application* CreateApplication(int argc, char** argv);
}
