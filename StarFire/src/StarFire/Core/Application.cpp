#include "sfpch.h"
#include "StarFire/Core/Application.h"
#include "StarFire/Core/Timestep.h"
#include "StarFire/Memory/RefRegistry.h"
#include "StarFire/Utility/Timer.h"

#include <thread>

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


		m_EventQueue = CreateScope<EventQueue>(100);
		RefRegistry::Init();

		WindowSpecification windowSpecs{};
		m_MainWindow = Window::Create(windowSpecs);
		SF_CORE_ASSERT(m_MainWindow != nullptr, "Unknown platform!");
		m_MainWindow->SetEventCallback(SF_BIND_EVENT_FN(Application::OnEvent));
		m_MainWindow->Init();

		RefRegistry::Get()->PrintRegister();

		Aurora::Log::SetCallback(SF_BIND_EVENT_FN(Application::RenderLogCallback));
		m_Aurora = CreateScope<Aurora::Renderer>();
		m_Aurora->Init("*wink*");


		SF_CORE_INFO("Application: Finished initialization.");
	}
	Application::~Application()
	{
		m_Aurora->Shutdown();
		m_MainWindow->Close();

		RefRegistry::Get()->PrintRegister();
		//LayerStack is cleaned automatically
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

	void Application::Run()
	{		
		SF_CORE_INFO("Starting main loop...");

		//temp
		std::thread appThread(SF_BIND_EVENT_FN(Application::AppLoop));
		while (m_Running)
		{			
			m_MainWindow->PollEvents();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		appThread.join();
		SF_CORE_INFO("Ending main loop...");	
	}

	void Application::AppLoop()
	{
		Utils::Timer timer;
		while (m_Running)
		{
			m_DeltaTimeInS = timer.Timestamp();
			if (!m_Minimized)
			{
				HandleUserInput();

				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(Timestep(m_DeltaTimeInS));
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
		}
	}

	void Application::HandleUserInput()
	{
		Scope<Event> e;
		while (m_EventQueue->Pop(e))
		{
			EventDispatcher dispatcher(*(e.get()));
			dispatcher.Dispatch<WindowCloseEvent>(SF_BIND_EVENT_FN(Application::OnWindowClose));
			dispatcher.Dispatch<WindowResizeEvent>(SF_BIND_EVENT_FN(Application::OnWindowResize));

			for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
			{
				(*it)->OnEvent(*(e.get()));
				if ((*(e.get())).Handled)
					break;
			}
		}
	}
	
	void Application::OnEvent(Scope<Event> e)
	{
		m_EventQueue->Push(std::move(e));
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

	void Application::RenderLogCallback(Aurora::LogLevel level, const std::string& msg)
	{
		switch (level)
		{
			case Aurora::LogLevel::ALL_TRACE: SF_R_CORE_TRACE(msg); break;
			case Aurora::LogLevel::ALL_INFO: SF_R_CORE_INFO(msg); break;
			case Aurora::LogLevel::ALL_DEBUG: SF_R_CORE_DEBUG_LOG(msg); break;
			case Aurora::LogLevel::ALL_WARN: SF_R_CORE_WARN(msg); break;
			case Aurora::LogLevel::ALL_ERROR: SF_R_CORE_ERROR(msg); break;
			case Aurora::LogLevel::ALL_CRITICAL: SF_R_CORE_CRITICAL(msg); break;
			default: SF_CORE_WARN("Unknown Aurora::LogLevel!"); break;
		}
	}

}
