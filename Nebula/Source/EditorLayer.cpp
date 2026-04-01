#include "EditorLayer.h"
#include "Profiling/Profiling.h"


namespace Nebula {

	EditorLayer::EditorLayer() : StarFire::Layer("Nebula - EditorLayer")
	{
		PROFILE_FUNCTION;
	}

	void EditorLayer::OnAttach()
	{
		PROFILE_FUNCTION;
	}

	void EditorLayer::OnDetach()
	{
		PROFILE_FUNCTION;
	}

	void EditorLayer::OnUpdate(StarFire::Timestep deltaTime)
	{
		PROFILE_FUNCTION;
	}

	void EditorLayer::OnGuiRender()
	{
		PROFILE_FUNCTION;
	}

	void EditorLayer::OnEvent(StarFire::Event& e)
	{
		PROFILE_FUNCTION;
		StarFire::EventDispatcher dispatcher(e);
	}

}