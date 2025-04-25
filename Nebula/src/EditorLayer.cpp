#include "EditorLayer.h"

#include <iostream>

namespace Nebula {

	EditorLayer::EditorLayer() : StarFire::Layer("Nebula - EditorLayer")
	{

	}

	void EditorLayer::OnAttach()
	{
		std::cout << "Attached layer " << m_DebugName << "!" << std::endl;

	}

	void EditorLayer::OnDetach()
	{
		std::cout << "Detached layer " << m_DebugName << "!" << std::endl;

	}

	void EditorLayer::OnUpdate(double deltaTime)
	{
		std::cout << "Hello World from " << m_DebugName << "!" << std::endl;
		std::cin.get();
	}

	void EditorLayer::OnGuiRender()
	{

	}
	
}