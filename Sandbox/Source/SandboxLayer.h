#pragma once
#include <StarFire.h>

namespace Sandbox {

	class SandboxLayer : public StarFire::Layer
	{
	public:
		SandboxLayer();
		~SandboxLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(StarFire::Timestep deltaTime) override;
		virtual void OnGuiRender() override;

		virtual void OnEvent(StarFire::Event& e) override;

	private:
		void Test();

	private:

	};
	
}