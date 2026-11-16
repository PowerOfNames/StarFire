#pragma once
#include "StarFire.h"
#include "Aurora/Renderer/RenderGraph.h"
#include "Aurora/Renderer/RenderPass.h"
#include "Substrate/RefPtr.h"
#include "CrudeCamera.h"

#include <glm/glm.hpp>

namespace Sandbox {

	struct SandboxLayerSpecification
	{
		uint32_t ViewportWidth = 800;
		uint32_t ViewportHeight = 600;
	};

	class SandboxLayer : public StarFire::Layer
	{
	public:
		SandboxLayer(const SandboxLayerSpecification& specs);
		~SandboxLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(StarFire::Timestep deltaTime) override;
		virtual void OnGuiRender() override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(StarFire::Event& e) override;

	private:
		// == Events ==
		bool OnFramebufferResize(StarFire::FramebufferResizeEvent& e);
		bool OnKeyPressed(StarFire::KeyPressedEvent& e);

		// == Others ==
		void HandleImGuiRenderTargetLogic(bool imGuiAvailable, bool imGuiActive);
	private:
		SandboxLayerSpecification m_Specification{};

		bool m_OpenDemoWindow = true;

		CrudeCamera m_Camera;

		Scope<StarFire::Scene> m_Scene;
		StarFire::Entity m_TriangleEntity;

		double m_CumulativeTimeInMS = 0.0;
		uint32_t m_FramebufferWidth = 0;
		uint32_t m_FramebufferHeight = 0;
		bool m_FramebufferResized = false;

		Ref<Aurora::RenderGraph> m_DefaultRenderGraph;
		Ref<Aurora::RenderPass> m_TrianglePass;

		struct ViewportPanel
		{
			glm::vec2 Bounds[2] = { {0.0f, 0.0f,}, {1.0f, 1.0f} };
			uint32_t Width = 0;
			uint32_t Height = 0;
			bool IsFocused = false;
			bool IsHovered = false;
		};
		ViewportPanel m_ViewportPanel;
		bool m_ViewportResized = false;

		Aurora::VertexBufferHandle m_TriangleVertexBufferHandle;
		Aurora::IndexBufferHandle m_TriangleIndexBufferHandle;
		Ref<Aurora::GraphicsPipeline> m_BindlessPipeline;

		uint32_t m_CheckerBoard[4] = { 0xFF00FF00, 0xFFFF00FF, 0xFF00FFFF, 0xFF00FF00 };
		Aurora::ImageHandle m_CheckerBoardTexture;

		const std::string c_DefaultPassName = "TrianglePass";
		const std::string c_DefaultColorAttachmentName = "TriangleColorAttachment";
		const std::string c_CopyColorTargetName = "TriangleColorAttachmentCopy";
	};
	
}