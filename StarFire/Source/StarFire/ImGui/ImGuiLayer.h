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

		void BlockEvents(bool state) { m_BlockEvents = state; }
		bool IsActive() const { return m_IsActive; }
		void SetActive(bool state) { m_IsActive = state; }

		const Ref<Aurora::ImGuiRenderer> GetImGuiRenderer() const { return m_ImGuiRenderer;	}

	private:
		bool OnFramebufferResize(FramebufferResizeEvent& e);

	private:
		bool m_BlockEvents = false;
		bool m_IsActive = true;

		Ref<Aurora::ImGuiRenderer> m_ImGuiRenderer = nullptr;
	};
}