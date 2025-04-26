#pragma once
#include <string>

#include "StarFire/Core/Core.h"
#include "StarFire/Events/ApplicationEvent.h"
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

		void OnEvent(Event& e);

		void PushOverlay(Layer* overlay);
		void PushLayer(Layer* layer);

		void Close();


		inline static Application* Get() { return s_Instance; }
		inline ApplicationSpecification& GetSpecification() { return m_Specification; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		static Application* s_Instance;

		bool m_Running = true;
		bool m_Minimized = false;

		LayerStack m_LayerStack;
	};

	Application* CreateApplication(int argc, char** argv);
}
