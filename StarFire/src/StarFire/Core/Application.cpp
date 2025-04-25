#include "sfpch.h"
#include "StarFire/Core/Application.h"

#include "StarFire/Core/Logging.h"

#include <GLFW/glfw3.h>

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
		SF_CORE_DEBUG("LogTest: Debug!");

		s_Instance = this;
		SF_CORE_INFO("Application: Finished initialization.");
	}
	Application::~Application()
	{
	}


	void Application::Run()
	{
		if (!glfwInit())
		{
			// Handle initialization failure
			std::cout << "Failed to initialize GLFW" << std::endl;
			SF_CORE_ERROR("Failed to initialize GLFW!");
		}
		SF_CORE_INFO("Initialized GLFW (Version {}.{}.{})", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
				
		GLFWwindow* m_Window = glfwCreateWindow(640, 480, m_Specification.Name.c_str(), NULL, NULL);

		SF_CORE_INFO("Starting main loop...");
		while (m_Running)
		{

			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate(0.16);
			}

			for (Layer* layer : m_LayerStack)
			{
				layer->OnGuiRender();
			}			
		}
		SF_CORE_INFO("Ending main loop...");
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		SF_CORE_WARN("Pushing {}", overlay->GetDebugName());

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::PushLayer(Layer* layer)
	{
		SF_CORE_WARN("Pushing {}", layer->GetDebugName());

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::Close()
	{
		SF_CORE_INFO("Closing...");
		m_Running = false;		
	}
}
