#include "SandboxLayer.h"


#include <iostream>

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


	void SandboxLayer::OnUpdate(double deltaTime)
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
