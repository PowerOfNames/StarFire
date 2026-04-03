#include "StarFire/Core/Layer.h"

namespace StarFire {


	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void PreUpdate(Timestep ts) override;
		virtual void OnUpdate(Timestep ts) override {};
		virtual void PostUpdate(Timestep ts) override;
		
		virtual void OnEvent(Event& e) override;

	private:
		bool m_BlockEvents = true;
	};
}