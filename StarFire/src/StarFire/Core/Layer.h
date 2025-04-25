#pragma once

#include <string>
#include <string_view>

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


		inline std::string_view GetDebugName() { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}
