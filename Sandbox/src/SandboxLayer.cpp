#include "SandboxLayer.h"

namespace Sandbox {

	SandboxLayer::SandboxLayer() : StarFire::Layer("SandboxLayer")
	{
	}

	void SandboxLayer::OnAttach()
	{
	}


	void SandboxLayer::OnDetach()
	{
	}


	void SandboxLayer::OnUpdate(StarFire::Timestep deltaTime)
	{
	}

	void SandboxLayer::OnGuiRender()
	{
	}	

	void SandboxLayer::OnEvent(StarFire::Event& e)
	{
		StarFire::EventDispatcher dispatcher(e);
	}
}
