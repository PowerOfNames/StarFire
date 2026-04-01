#include "SandboxLayer.h"
#include "Profiling/Profiling.h"

namespace Sandbox {

	SandboxLayer::SandboxLayer() : StarFire::Layer("SandboxLayer")
	{
		PROFILE_FUNCTION;

	}

	void SandboxLayer::OnAttach()
	{
		PROFILE_FUNCTION;

	}


	void SandboxLayer::OnDetach()
	{
		PROFILE_FUNCTION;

	}


	void SandboxLayer::OnUpdate(StarFire::Timestep deltaTime)
	{
		PROFILE_FUNCTION;

	}

	void SandboxLayer::OnGuiRender()
	{
		PROFILE_FUNCTION;

	}	

	void SandboxLayer::OnEvent(StarFire::Event& e)
	{
		PROFILE_FUNCTION;

		StarFire::EventDispatcher dispatcher(e);
	}
}
