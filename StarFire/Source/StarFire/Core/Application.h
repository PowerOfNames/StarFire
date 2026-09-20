#pragma once
#include "StarFire/Core/Core.h"
#include "StarFire/Core/LayerStack.h"
#include "StarFire/Core/Window.h"
#include "StarFire/Events/ApplicationEvent.h"
#include "StarFire/Events/EventQueue.h"

#include <string>
#include <atomic>

namespace StarFire {

	struct ApplicationSpecification
	{
		std::string Name;
		std::string RootPath; //empty -> executable dir
		uint64_t MaxFrames = UINT64_MAX;
		bool UseImGui = false;

		struct ApplicationWindowSpecification
		{			
			uint32_t Width = 1600;
			uint32_t Height = 900;
			bool Fullscreen = false;
		}WindowSpecs;

		//Debugging
	};

	class ImGuiLayer;

	class Application
	{
	public:
		Application(const ApplicationSpecification& specs);
		virtual ~Application();

		bool Init();
		void Run();

		void OnEvent(Scope<Event> e);

		void PushOverlay(Layer* overlay);
		void PushLayer(Layer* layer);

		//Call from the code to stop the update loop
		void Close();

		inline ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer;	}

		inline static Application* Get() { return s_Instance; }
		inline ApplicationSpecification& GetSpecification() { return m_Specification; }

		inline Window* GetMainWindowPtr() const { return m_MainWindow.get(); }

	protected:
		virtual void OnInit() = 0;

	private:
		void HandleUserInput();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnFramebufferResize(FramebufferResizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		static Application* s_Instance;
				
		Scope<Window> m_MainWindow = nullptr;
		Scope<EventQueue> m_EventQueue = nullptr;

		ImGuiLayer* m_ImGuiLayer = nullptr;

		double m_DeltaTimeInS = 0.0;

		std::atomic_bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
	};

	Application* CreateApplication(int argc, char** argv);
}
