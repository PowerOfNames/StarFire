#include "SandboxLayer.h"

#include <iostream>

namespace Sandbox {

	SandboxLayer::SandboxLayer() : StarFire::Layer("SandboxLayer")
	{
	}

	void SandboxLayer::OnAttach()
	{
		std::cout << "Attached layer " << m_DebugName << "!" << std::endl;
	}


	void SandboxLayer::OnDetach()
	{
		std::cout << "Detached layer " << m_DebugName << "!" << std::endl;
	}


	void SandboxLayer::OnUpdate(double deltaTime)
	{
		std::cout << "Hello World from " << m_DebugName << "!" << std::endl;
		std::cin.get();
	}

	void SandboxLayer::OnGuiRender()
	{

	}	
}
