#include "sfpch.h"
#include "StarFire/Profiling/Profiling.h"

#include "StarFire/Core/Application.h"
#include "StarFire/Core/FileSystem.h"
#include "StarFire/Core/Timestep.h"
#include "StarFire/Memory/RefRegistry.h"
#include "StarFire/Utility/Timer.h"
#include "StarFire/ImGui/ImGuiLayer.h"

#include <Aurora/Aurora.h>
#include <Aurora/Logging/LogLevel.h>

namespace StarFire {

	
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specs)
		: m_Specification(specs)
	{
		PROFILE_FUNCTION;
			
		STARFIRE_TRACE("Application: Starting initialization...");

		s_Instance = this;		
	}
	Application::~Application()
	{
		PROFILE_FUNCTION;

		m_LayerStack.Clear();
		Aurora::Shutdown();
		if (m_MainWindow)
			m_MainWindow->Close();
		m_MainWindow = nullptr;

		RefRegistry::Get()->PrintRegister();
	}

	bool Application::Init()
	{
		// ========== Application Setup ==========		
		if (!FileSystem::Instance()->SetProjectDir(std::filesystem::path(m_Specification.ProjectPath)))
		{
			STARFIRE_CRITICAL("Failed to set project path.");
			return false;
		}
		
		STARFIRE_INFO("Filesystem initialized for Project \"{}\"", FileSystem::Instance()->GetProjectName());

		m_EventQueue = CreateScope<EventQueue>(100);

		WindowSpecification windowSpecs{};
		windowSpecs.Title = FileSystem::Instance()->GetProjectName();
		windowSpecs.Width = m_Specification.WindowSpecs.Width;
		windowSpecs.Height = m_Specification.WindowSpecs.Height;
		windowSpecs.Fullscreen = m_Specification.WindowSpecs.Fullscreen;
		m_MainWindow = Window::Create(windowSpecs);
		if (m_MainWindow == nullptr)
		{
			STARFIRE_CRITICAL("Failed to create window!");
			return false;
		}
		m_MainWindow->SetEventCallback(STARFIRE_BIND_EVENT_FN(Application::OnEvent));
		if (!m_MainWindow->Init())
		{
			STARFIRE_CRITICAL("Failed to initialize window!");
			return false;
		}

		// ========== Renderer Setup ==========
		Aurora::SetRefRegistryRegisterCallback([](const std::string& typeName, std::atomic<uint64_t>* counter)
			{
				RefRegistry::Get()->Register(typeName, counter);
			});
		Aurora::SetRefRegistryUnregisterCallback([](const std::string& typeName)
			{
				RefRegistry::Get()->Unregister(typeName);
			});


		Aurora::SetLoggingCallback([](Aurora::LogLevel level, const std::string& msg, const char* file, const char* func, int line)
			{
				switch (level)
				{
					case Aurora::LogLevel::LOG_LEVEL_TRACE: RENDERER_TRACE_HOOK(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_INFO: RENDERER_INFO_HOOK(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_DEBUG: RENDERER_DEBUG_HOOK(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_WARN: RENDERER_WARN_HOOK(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_ERROR: RENDERER_ERROR_HOOK(file, func, line, msg); break;
					case Aurora::LogLevel::LOG_LEVEL_CRITICAL: RENDERER_CRITICAL_HOOK(file, func, line, msg); break;
					default: STARFIRE_VALIDATE(false, "Unknown Aurora::LogLevel!"); break;
				}
			});
		
		auto& appSettings = Aurora::ChangeAppSettings();
		if (!appSettings.SetCacheRootDir(FileSystem::Instance()->GetCacheRootDir()))
		{
			STARFIRE_CRITICAL("Failed to set cache root dir for Aurora.");
			return false;
		}

		if (!appSettings.SetShaderCacheDir(FileSystem::Instance()->GetShaderCacheDir()))
		{
			STARFIRE_CRITICAL("Failed to set shader cache dir for Aurora.");
			return false;
		}

		Aurora::InitializationSpecification initSpecs{};
		initSpecs.AppName = m_Specification.Name;
		initSpecs.AppVersion = { 1, 0, 0 };
		initSpecs.AuroraVersion = { 1, 0, 0 };
		initSpecs.InstanceSpecs.EnableDebugUtils = true;
		initSpecs.SurfaceSpecs.WSI = Aurora::WSIPlatformType::SURFACE_PLATFORM_GLFW;
		initSpecs.SurfaceSpecs.WindowHandle = m_MainWindow->GetNativeWindow();
		initSpecs.SurfaceSpecs.FramesPerFlight = 2;
		initSpecs.SurfaceSpecs.VSync = false;
		initSpecs.SurfaceSpecs.Width = m_MainWindow->GetWidth();
		initSpecs.SurfaceSpecs.Height = m_MainWindow->GetHeight();
		initSpecs.SurfaceSpecs.FramebufferWidth = m_MainWindow->GetFramebufferWidth();
		initSpecs.SurfaceSpecs.FramebufferHeight = m_MainWindow->GetFramebufferHeight();
		initSpecs.SurfaceSpecs.ClearColor = { 0.5f, 0.0f, 0.0f, 1.0f };
		if (!Aurora::Initialize(initSpecs))
		{
			STARFIRE_CRITICAL("Failed to initialize Aurora.");
			return false;
		}

		// ========== ImGui setup ==========
		if (m_Specification.UseImGui)
		{
			m_ImGuiLayer = new ImGuiLayer();
			PushOverlay(m_ImGuiLayer);
		}

		//bool is = std::filesystem::path("blob.ext") == "blob.ext";
		//auto ext = std::filesystem::path("blob.frag.spv").extension();

		RefRegistry::Get()->PrintRegister();

		STARFIRE_TRACE("Application: Finished initialization.");
		OnInit();
		return true;
	}

	void Application::PushOverlay(Layer* overlay)
	{
		PROFILE_FUNCTION;

		STARFIRE_INFO("Pushing {}", overlay->GetDebugName());

		m_LayerStack.PushOverlay(overlay);
	}

	void Application::PushLayer(Layer* layer)
	{
		PROFILE_FUNCTION;

		STARFIRE_INFO("Pushing {}", layer->GetDebugName());

		m_LayerStack.PushLayer(layer);
	}

	void Application::Close()
	{
		PROFILE_FUNCTION;

		STARFIRE_INFO("Closing...");
		m_Running = false;
	}

	void Application::Run()
	{
		PROFILE_FUNCTION;
		
		PROFILE_THREAD_NAME("Main Thread", 0);

		STARFIRE_TRACE("Starting main loop...");
		Utils::Timer timer;
		while (m_Running)
		{
			if (Aurora::GetTotalFrameCount() > m_Specification.MaxFrames)
				Close();

			PROFILE_SCOPE("Frame loop");
			m_MainWindow->PollEvents();
			m_DeltaTimeInS = timer.Timestamp();
			HandleUserInput();

			if (m_Minimized)
				STARFIRE_INFO("Application minimized");


			if (!Aurora::BeginFrame())
				continue;

			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate(Timestep(m_DeltaTimeInS));
			}

			if (m_ImGuiLayer)
				m_ImGuiLayer->BeginFrame();

			for (Layer* layer : m_LayerStack)
			{
				layer->OnGuiRender();
			}

			if (m_ImGuiLayer)
				m_ImGuiLayer->EndFrame();

			Aurora::EndFrame();
			Aurora::SwapFrame();

			PROFILE_FRAME_MARK;
		}

		//appThread.join();
		STARFIRE_TRACE("Ending main loop...");	
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
			dispatcher.Dispatch<WindowCloseEvent>(STARFIRE_BIND_EVENT_FN(Application::OnWindowClose));
			dispatcher.Dispatch<WindowResizeEvent>(STARFIRE_BIND_EVENT_FN(Application::OnWindowResize));
			dispatcher.Dispatch<FramebufferResizeEvent>(STARFIRE_BIND_EVENT_FN(Application::OnFramebufferResize));

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

		STARFIRE_DEBUG("Closing window...");
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
		
		STARFIRE_DEBUG("Window resize to [{}|{}]", newWidth, newHeight);

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
		STARFIRE_DEBUG("Framebuffer resize to to [{}|{}]", newWidth, newHeight);
		Aurora::Resize(newWidth, newHeight);

		return false;
	}
}
