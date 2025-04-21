#include "sfpch.h"
#include "StarFire/Core/Application.h"


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
		while (m_Running)
		{
			std::cout << "Hello World from " << m_Specification.Name << "!" << std::endl;
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}
}
