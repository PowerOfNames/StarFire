#include "sfpch.h"
#include "StarFire/Profiling/Profiling.h"

#include "StarFire/Core/Application.h"
#include "StarFire/Core/Timestep.h"
#include "StarFire/Memory/RefRegistry.h"
#include "StarFire/Utility/Timer.h"


#include <Aurora/Aurora.h>
#include <Aurora/Logging/LogLevel.h>


#include <thread>

namespace StarFire {

	
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specs)
		: m_Specification(specs)
	{
		PROFILE_FUNCTION;
			
		SF_CORE_TRACE("Application: Starting initialization...");


		s_Instance = this;

		m_EventQueue = CreateScope<EventQueue>(100);
		RefRegistry::Init();


		WindowSpecification windowSpecs{};
		m_MainWindow = Window::Create(windowSpecs);
		SF_CORE_ASSERT(m_MainWindow != nullptr, "Unknown platform!");
		m_MainWindow->SetEventCallback(SF_BIND_EVENT_FN(Application::OnEvent));
		m_MainWindow->Init();


		Aurora::SetLoggingCallback([](Aurora::LogLevel level, const std::string& msg, const char* file, const char* func, int line)
			{
				switch (level)
				{
					case Aurora::LogLevel::LOG_LEVEL_TRACE: SF_R_CORE_TRACE(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_INFO: SF_R_CORE_INFO(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_DEBUG: SF_R_CORE_DEBUG(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_WARN: SF_R_CORE_WARN(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_ERROR: SF_R_CORE_ERROR(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_CRITICAL: SF_R_CORE_CRITICAL(file, func, line, msg); break;
					default: SF_CORE_WARN("Unknown Aurora::LogLevel!"); break;
				}
			});
		Aurora::SetRefRegistryRegisterCallback([](const std::string& typeName, std::atomic<uint64_t>* counter)
			{
				RefRegistry::Get()->Register(typeName, counter);
			});
		Aurora::SetRefRegistryUnregisterCallback([](const std::string& typeName)
			{
				RefRegistry::Get()->Unregister(typeName);
			});
		
		Aurora::RenderContextSpecification renderSpecs{};
		renderSpecs.AppName = m_Specification.Name;
		renderSpecs.AppVersion = { 1, 0, 0 };
		renderSpecs.AuroraVersion = { 1, 0, 0 };
		renderSpecs.InstanceSpecs.EnableDebugUtils = true;
		renderSpecs.SurfaceSpecs.WSI = Aurora::WSIPlatformType::SURFACE_PLATFORM_GLFW;
		renderSpecs.SurfaceSpecs.WindowHandle = m_MainWindow->GetNativeWindow();
		renderSpecs.SurfaceSpecs.FramesPerFlight = 2;
		renderSpecs.SurfaceSpecs.VSync = false;
		renderSpecs.SurfaceSpecs.Width = m_MainWindow->GetWidth();
		renderSpecs.SurfaceSpecs.Height = m_MainWindow->GetHeight();
		renderSpecs.SurfaceSpecs.FramebufferWidth = m_MainWindow->GetFramebufferWidth();
		renderSpecs.SurfaceSpecs.FramebufferHeight = m_MainWindow->GetFramebufferHeight();
		renderSpecs.SurfaceSpecs.ClearColor = { 0.5f, 0.0f, 0.0f, 1.0f };
		Aurora::InitializeRenderContext(renderSpecs);
		
		// ========== Register Resources ==========
		auto& appSettings = Aurora::ChangeAppSettings();
		appSettings.SetRootPath(std::filesystem::current_path());


		bool is = std::filesystem::path("blob.ext") == "blob.ext";
		auto ext = std::filesystem::path("blob.frag.spv").extension();

		RefRegistry::Get()->PrintRegister();


		SF_CORE_TRACE("Application: Finished initialization.");
	}
	Application::~Application()
	{
		PROFILE_FUNCTION;

		Aurora::Shutdown();
		m_MainWindow->Close();

		RefRegistry::Get()->PrintRegister();
		//LayerStack is cleaned automatically
	}

	void Application::PushOverlay(Layer* overlay)
	{
		PROFILE_FUNCTION;

		SF_CORE_INFO("Pushing {}", overlay->GetDebugName());

		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PushLayer(Layer* layer)
	{
		PROFILE_FUNCTION;

		SF_CORE_INFO("Pushing {}", layer->GetDebugName());

		m_LayerStack.PushLayer(layer);
	}

	void Application::Close()
	{
		PROFILE_FUNCTION;

		SF_CORE_INFO("Closing...");
		m_Running = false;
	}

	void Application::Run()
	{
		PROFILE_FUNCTION;
		PROFILE_THREAD_NAME("Main Thread", 0);


		SF_CORE_TRACE("Starting main loop...");

		//temp
		std::thread appThread(SF_BIND_EVENT_FN(Application::AppLoop));
		while (m_Running)
		{			
			PROFILE_SCOPE("Event polling loop");


			m_MainWindow->PollEvents();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		appThread.join();
		SF_CORE_TRACE("Ending main loop...");	
	}

	void Application::AppLoop()
	{
		PROFILE_FUNCTION;
		PROFILE_THREAD_NAME("Render Thread", 1);

		Utils::Timer timer;
		while (m_Running)
		{
			PROFILE_SCOPE("Frame loop");

			m_DeltaTimeInS = timer.Timestamp();
			HandleUserInput();
			if (!m_Minimized)
			{

				if(!Aurora::BeginFrame())
					continue;

				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(Timestep(m_DeltaTimeInS));
				}
				Aurora::EndFrame();

				//Todo: add Aurora::BeginUiFrame
				for (Layer* layer : m_LayerStack)
				{
					layer->OnGuiRender();
				}
				//Todo: add Aurora::EndUiFrame

				Aurora::SwapFrame();
			}
			else
			{
				SF_CORE_INFO("Application minimized");
			}
			PROFILE_FRAME_MARK;
		}
		SF_CORE_WARN("Leaving main loop!");
	}

	void Application::HandleUserInput()
	{
		PROFILE_FUNCTION;

		Scope<Event> e;
		m_EventQueue->GatherCoalescing();
		while (m_EventQueue->Pop(e))
		{
			PROFILE_SCOPE("Handle Event");

			EventDispatcher dispatcher(*(e.get()));
			dispatcher.Dispatch<WindowCloseEvent>(SF_BIND_EVENT_FN(Application::OnWindowClose));
			dispatcher.Dispatch<WindowResizeEvent>(SF_BIND_EVENT_FN(Application::OnWindowResize));
			dispatcher.Dispatch<FramebufferResizeEvent>(SF_BIND_EVENT_FN(Application::OnFramebufferResize));

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
		PROFILE_FUNCTION;

		m_EventQueue->Push(std::move(e));
	}	

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		PROFILE_FUNCTION;

		SF_CORE_DEBUG("Closing window...");
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PROFILE_FUNCTION;

		uint32_t newWidth = e.GetWidth();
		uint32_t newHeight = e.GetHeight();
		if (newWidth == 0 || newHeight == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		SF_CORE_DEBUG("Window resize to to [{}|{}]", newWidth, newHeight);

		return false;
	}

	bool Application::OnFramebufferResize(FramebufferResizeEvent& e)
	{
		PROFILE_FUNCTION;

		uint32_t newWidth = e.GetWidth();
		uint32_t newHeight = e.GetHeight();
		if (newWidth == 0 || newHeight == 0)
		{
			return false;
		}
		SF_CORE_DEBUG("Framebuffer resize to to [{}|{}]", newWidth, newHeight);
		Aurora::Resize(newWidth, newHeight);

		return false;
	}
}
