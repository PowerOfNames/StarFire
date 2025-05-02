#include "sfpch.h"
#include "StarFire/Core/Application.h"
#include "StarFire/Utility/Timer.h"

namespace StarFire {

	
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specs)
		: m_Specification(specs)
	{
		SF_CORE_INFO("Application: Starting initialization...");
		SF_CORE_TRACE("LogTest: Trace!");
		SF_CORE_INFO("LogTest: Info!");
		SF_CORE_WARN("LogTest: Warn!");
		SF_CORE_ERROR("LogTest: Error!");
		SF_CORE_CRITICAL("LogTest: Critical!");
		SF_CORE_DEBUG_LOG("LogTest: Debug!");

		s_Instance = this;


		WindowSpecification windowSpecs{};
		m_MainWindow = Window::Create(windowSpecs);
		SF_CORE_ASSERT(m_MainWindow != nullptr, "Unknown platform!");
		m_MainWindow->SetEventCallback(SF_BIND_EVENT_FN(Application::OnEvent));
		m_MainWindow->Init();


		SF_CORE_INFO("Application: Finished initialization.");
	}
	Application::~Application()
	{
		m_MainWindow->Close();
		//LayerStack is cleaned automatically
	}	

	void Application::Run()
	{		
		SF_CORE_INFO("Starting main loop...");
		Utils::Timer timer;
		while (m_Running)
		{
			double deltaTime = timer.Timestamp();


			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(deltaTime);
				}

				for (Layer* layer : m_LayerStack)
				{
					layer->OnGuiRender();
				}
			
				m_MainWindow->OnUpdate();
			}
			else
			{
				SF_CORE_INFO("Application minimized");
			}

			m_MainWindow->PollEvents();
		}
		SF_CORE_INFO("Ending main loop...");		
	}

	void Application::PushOverlay(Layer* overlay)
	{
		SF_CORE_WARN("Pushing {}", overlay->GetDebugName());

		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PushLayer(Layer* layer)
	{
		SF_CORE_WARN("Pushing {}", layer->GetDebugName());

		m_LayerStack.PushLayer(layer);
	}

	void Application::Close()
	{
		SF_CORE_INFO("Stopping update loop...");
		m_Running = false;
	}
	
	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(SF_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(SF_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		SF_CORE_DEBUG_LOG("Closing window...");
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		SF_CORE_DEBUG_LOG("Window resize to to [{}|{}]", e.GetWidth(), e.GetHeight());

		return false;
	}
}
