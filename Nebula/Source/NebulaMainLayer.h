#pragma once
#include <StarFire.h>

namespace Nebula {

	class NebulaMainLayer : public StarFire::Layer
	{
	public:
		NebulaMainLayer();
		~NebulaMainLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(StarFire::Timestep deltaTime) override;
		virtual void OnGuiRender() override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(StarFire::Event& e) override;

	private:

	};
}
