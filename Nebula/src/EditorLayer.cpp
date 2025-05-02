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

	void EditorLayer::OnUpdate(StarFire::Timestep deltaTime)
	{		
	}

	void EditorLayer::OnGuiRender()
	{
	}

	void EditorLayer::OnEvent(StarFire::Event& e)
	{
		StarFire::EventDispatcher dispatcher(e);
	}

}