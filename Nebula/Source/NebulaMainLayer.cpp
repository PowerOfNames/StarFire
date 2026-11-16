#include "NebulaMainLayer.h"
#include "Profiling/Profiling.h"


namespace Nebula {

	NebulaMainLayer::NebulaMainLayer() : StarFire::Layer("Nebula - EditorLayer")
	{
		PROFILE_FUNCTION;
	}

	void NebulaMainLayer::OnAttach()
	{
		PROFILE_FUNCTION;
	}

	void NebulaMainLayer::OnDetach()
	{
		PROFILE_FUNCTION;
	}

	void NebulaMainLayer::OnUpdate(StarFire::Timestep deltaTime)
	{
		PROFILE_FUNCTION;
	}

	void NebulaMainLayer::OnGuiRender()
	{
		PROFILE_FUNCTION;
	}

	void NebulaMainLayer::OnImGuiRender()
	{
		PROFILE_FUNCTION;
	}

	void NebulaMainLayer::OnEvent(StarFire::Event& e)
	{
		PROFILE_FUNCTION;
		StarFire::EventDispatcher dispatcher(e);
	}

}