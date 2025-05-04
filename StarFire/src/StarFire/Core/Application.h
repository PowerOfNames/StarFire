#pragma once
#include "StarFire/Events/ApplicationEvent.h"
#include "StarFire/Core/Core.h"
#include "StarFire/Core/LayerStack.h"
#include "StarFire/Core/Window.h"


#include <string>

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

		//Call from the code to stop the update loop
		void Close();


		inline static Application* Get() { return s_Instance; }
		inline ApplicationSpecification& GetSpecification() { return m_Specification; }

		inline Window* GetMainWindowPtr() const { return m_MainWindow.get(); }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		static Application* s_Instance;

		Scope<Window> m_MainWindow = nullptr;

		double m_DeltaTimeInS = 0.0;

		bool m_Running = true;
		bool m_Minimized = false;

		LayerStack m_LayerStack;
	};

	Application* CreateApplication(int argc, char** argv);
}
