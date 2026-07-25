#include "StarFire/Core/Layer.h"
#include "StarFire/Core/Timestep.h"
#include "StarFire/Events/Event.h"
#include "StarFire/Events/ApplicationEvent.h"

#include "Aurora/ImGui/ImGuiRenderer.h"

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

		const Ref<Aurora::ImGuiRenderer> GetImGuiRenderer() const { return m_ImGuiRenderer;	}

	private:
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		bool m_BlockEvents = true;

		Ref<Aurora::ImGuiRenderer> m_ImGuiRenderer = nullptr;
	};
}