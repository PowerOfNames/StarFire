#pragma once
#include "StarFire.h"
#include "Substrate/RefPtr.h"

#include <glm/glm.hpp>

namespace Sandbox {

	class SandboxLayer : public StarFire::Layer
	{
	public:
		SandboxLayer();
		~SandboxLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(StarFire::Timestep deltaTime) override;
		virtual void OnGuiRender() override;

		virtual void OnEvent(StarFire::Event& e) override;

	private:
		void Test();

	private:
		bool m_OpenDemoWindow = true;

		Ref<Aurora::RenderGraph> m_DefaultRenderGraph;
		Ref<Aurora::RenderPass> m_TrianglePass;
		Aurora::Shape2DHandle m_TriangleShapeHandle;
		Aurora::MaterialHandle m_TriangleColorMaterialHandle;

		struct ViewportPanel
		{
			glm::vec2 Bounds[2];
			uint32_t Width;
			uint32_t Height;
			bool IsFocused = false;
			bool IsHovered = false;
		};
		ViewportPanel m_ViewportPanel;

		Aurora::VertexBufferHandle m_TriangleVertexBufferHandle;
		Aurora::IndexBufferHandle m_TriangleIndexBufferHandle;

		const std::string c_DefaultPassName = "TrianglePass";
		const std::string c_DefaultColorAttachmentName = "TriangleColorAttachment";
		const std::string c_CopyColorTargetName = "TriangleColorAttachmentCopy";
	};
	
}