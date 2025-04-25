#include "EditorLayer.h"

#include <iostream>

namespace Nebula {

	EditorLayer::EditorLayer() : StarFire::Layer("Nebula - EditorLayer")
	{

	}

	void EditorLayer::OnAttach()
	{

	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(double deltaTime)
	{
		std::cin.get();
	}

	void EditorLayer::OnGuiRender()
	{

	}
	
}