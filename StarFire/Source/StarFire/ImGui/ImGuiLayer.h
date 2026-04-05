#include "StarFire/Core/Layer.h"
#include "StarFire/Core/Timestep.h"
#include "StarFire/Events/Event.h"

namespace StarFire {


	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override {};
		
		virtual void OnEvent(Event& e) override;

		void BeginFrame() const;
		void EndFrame() const;

	private:
		bool m_BlockEvents = true;
	};
}