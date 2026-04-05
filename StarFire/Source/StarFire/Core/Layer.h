#pragma once
#include "StarFire/Core/Timestep.h"
#include "StarFire/Events/Event.h"

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

		virtual void OnUpdate(Timestep deltaTime) {};
		
		virtual void OnGuiRender() {};

		virtual void OnEvent(Event& event) {};

		inline std::string_view GetDebugName() { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}
