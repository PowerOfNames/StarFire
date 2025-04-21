#include "sfpch.h"
#include "StarFire/Core/Application.h"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace StarFire {


	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specs)
		: m_Specification(specs)
	{
		s_Instance = this;
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
		}
		std::cout << "Successfully initialized GLFW, version: "
			<< GLFW_VERSION_MAJOR << "." 
			<< GLFW_VERSION_MINOR << "." 
			<< GLFW_VERSION_REVISION 
			<< std::endl;

		spdlog::info("Info");
		spdlog::error("Error");
		spdlog::warn("Warn");
		spdlog::critical("Critical");
		
		spdlog::debug("Debug");
		spdlog::set_level(spdlog::level::debug);
		spdlog::debug("Debug");

		
		GLFWwindow* m_Window = glfwCreateWindow(640, 480, m_Specification.Name.c_str(), NULL, NULL);

		while (m_Running)
		{
			std::cout << "Hello World from " << m_Specification.Name << "!" << std::endl;
			std::cin.get();
		}
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Application::Close()
	{
		m_Running = false;
	}
}
