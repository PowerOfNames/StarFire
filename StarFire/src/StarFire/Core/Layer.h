#pragma once

#include <string>

namespace StarFire {

	class Layer
	{
	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate(double deltaTime) {};
		virtual void OnGuiRender() {};

	protected:
		std::string m_DebugName;
	};
}
