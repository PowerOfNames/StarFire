#pragma once
#include <StarFire.h>

namespace Nebula {

	class EditorLayer : public StarFire::Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnGuiRender() override;

	private:

	};
}
