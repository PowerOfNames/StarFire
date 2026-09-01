#include "SandboxLayer.h"
#include "Profiling/Profiling.h"

#include "StarFire.h"
#include "Aurora/Aurora.h"
#include "Aurora/Renderer/RenderGraph.h"
#include "Aurora/Renderer/RenderPass.h"


#include <imgui.h>


namespace Sandbox {

	SandboxLayer::SandboxLayer() : StarFire::Layer("SandboxLayer")
	{
		PROFILE_FUNCTION;

		Aurora::RenderGraphSpecification rgSpecs{};
		rgSpecs.Name = "Triangle Test RG";
		m_DefaultRenderGraph = Aurora::RenderGraph::Create(rgSpecs);


		uint32_t width = 800;
		uint32_t height = 600;
		// == Simple 2D pass for triangle rendering as test ==
		{
			Aurora::RenderPassSpecification specs{};
			specs.Name = c_DefaultPassName;
			Aurora::ColorAttachmentSpecification colorAttachment{};
			colorAttachment.Name = c_DefaultColorAttachmentName;
			colorAttachment.ImageSpecs.Name = colorAttachment.Name + "_Image";
			colorAttachment.ImageSpecs.Usage = Aurora::ImageUsageFlags::COLOR_ATTACHMENT | Aurora::ImageUsageFlags::SAMPLED | Aurora::ImageUsageFlags::TRANSFER_SRC;
			colorAttachment.ImageSpecs.Format = Aurora::Format::RGBA8_UNORM;
			colorAttachment.ImageSpecs.Width = width;
			colorAttachment.ImageSpecs.Height = height;
			colorAttachment.ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
			specs.ColorAttachments.push_back(std::move(colorAttachment));

			specs.DepthAttachment.Name = "Depth";
			specs.DepthAttachment.ImageSpecs.Name = specs.DepthAttachment.Name + "_Image";
			specs.DepthAttachment.ImageSpecs.Usage = Aurora::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT | Aurora::ImageUsageFlags::TRANSFER_SRC;
			specs.DepthAttachment.ImageSpecs.Format = Aurora::Format::DEPTH32_SFLOAT;
			specs.DepthAttachment.ImageSpecs.Width = width;
			specs.DepthAttachment.ImageSpecs.Height = height;
			specs.DepthAttachment.ClearDepth = 1.0f; //needs to be 0.0 if we do infinite far plane, but for now we do a finite far plane so 1.0 is correct
			specs.RenderArea = { width, height };
			m_TrianglePass = Aurora::RenderPass::Create(specs);
		}

		m_DefaultRenderGraph->AddRenderPass(m_TrianglePass);

		
	}

	void SandboxLayer::OnAttach()
	{
		PROFILE_FUNCTION;

		// == 1. Compile and build al GPU resources now
		
		//This compiles the architecture given during creation and sets up everything. This should contain the complete capability of this render graph
		m_DefaultRenderGraph->Compile();
		Aurora::AttachmentCopyRequest colorAttachmentCopy{};
		colorAttachmentCopy.PassName = c_DefaultPassName;
		colorAttachmentCopy.AttachmentName = c_DefaultColorAttachmentName;
		m_DefaultRenderGraph->AddAttachmentCopy(c_CopyColorTargetName, colorAttachmentCopy);

		// == 2. Build scene ==
		// We do initial scene building here probably. Not sure if this is also the place to load assets etc for the given scene

		Aurora::VertexLayout vertexBufferLayout({
			{.Name = "a_Position", .Location = 0, .Type = Aurora::FieldType::POSITION, .Offset = offsetof(StarFire::Vertex, Position) },
			{.Name = "a_Color", .Location = 1, .Type = Aurora::FieldType::COLOR, .Offset = offsetof(StarFire::Vertex, Color) }
			});
		Aurora::VertexBufferSpecification vertexBufferSpecs{};
		vertexBufferSpecs.Name = "Triangle Vertex Buffer";
		vertexBufferSpecs.Layout = vertexBufferLayout;
		vertexBufferSpecs.Size = StarFire::TriangleVertices.size() * sizeof(StarFire::Vertex);
		vertexBufferSpecs.SpecializationType = Aurora::BufferSpecializationType::STATIC_VERTEX_BUFFER;
		vertexBufferSpecs.Data = (void*)StarFire::TriangleVertices.data();
		m_TriangleVertexBufferHandle = Aurora::CreateVertexBuffer(vertexBufferSpecs);
	}


	void SandboxLayer::OnDetach()
	{
		PROFILE_FUNCTION;

		m_DefaultRenderGraph->Destroy();
		m_DefaultRenderGraph = nullptr;
		m_TrianglePass = nullptr;
		Aurora::DestroyVertexBuffer(m_TriangleVertexBufferHandle);
		m_TriangleVertexBufferHandle = Aurora::VertexBufferHandle::INVALID_HANDLE;
		//Here the scene resources should be freed (or pushed into destruction queues, etc). This is also probably the place to save the scene if needed
	}


	void SandboxLayer::OnUpdate(StarFire::Timestep deltaTime)
	{
		PROFILE_FUNCTION;

		if (m_ViewportResized)
		{
			m_DefaultRenderGraph->OnResize(static_cast<uint32_t>(m_ViewportPanel.Width), static_cast<uint32_t>(m_ViewportPanel.Height));
			m_ViewportResized = false;
		}

		// == 2. Update Scene ==

		//StarFire::RendererAPI::DrawSprite(m_TriangleShapeHandle, m_TriangleTransform, m_TriangleColorMaterialHandle);

		// == 3. Update Physics ==

		// 2. and 3. should probably be just a call into the respective systems to kick their workers

		// == 4. Sync ==
		// If necessary, we must sync scene and physics now before rendering. This is also the place to do any late updates to the scene that must happen after physics, etc.

		// == 5. Render scene ==
		// Here we should probably just call the render graph to execute and it should take care of everything. We might need to do some per-frame setup for the render graph here (like culling), but that should be it.

		//Temp call, this should later consume the finished scene data and either do culling OR take the already culled scene
		m_DefaultRenderGraph->Execute(m_TriangleVertexBufferHandle, m_TriangleIndexBufferHandle);

	}

	void SandboxLayer::OnGuiRender()
	{
		PROFILE_FUNCTION;

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();



		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Test"))
			{
				if (ImGui::MenuItem("Nested Test 2", "Crt+P"))
					ImGui::Checkbox("Demo", &m_OpenDemoWindow);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_OpenDemoWindow)
			ImGui::ShowDemoWindow();

		// ===== Viewport =====
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		{
			ImGui::Begin("Viewport");
			ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
			ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();

			m_ViewportPanel.Bounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			m_ViewportPanel.Bounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

			m_ViewportPanel.IsFocused = ImGui::IsWindowFocused();
			m_ViewportPanel.IsHovered = ImGui::IsWindowHovered();

			float width = glm::max(m_ViewportPanel.Bounds[1].x - m_ViewportPanel.Bounds[0].x, 1.0f);
			float height = glm::max(m_ViewportPanel.Bounds[1].y - m_ViewportPanel.Bounds[0].y, 1.0f);
			
			//TODO: Use the viewport's focused/hovered state to control whether the camera controller should receive input, etc.
			Aurora::ImageHandle viewportImage = m_DefaultRenderGraph->GetCopyTarget(c_CopyColorTargetName);
			if(viewportImage != Aurora::ImageHandle::INVALID_HANDLE)
			{
				uint64_t textureId = StarFire::Application::Get()->GetImGuiLayer()->GetImGuiRenderer()->GetTextureIDFromHandle(viewportImage);

				if (m_ViewportPanel.Width != width || m_ViewportPanel.Height != height)
				{
					m_ViewportPanel.Width = width;
					m_ViewportPanel.Height = height;
					m_ViewportResized = true;
				}

				ImGui::Image(textureId, ImVec2{ m_ViewportPanel.Width, m_ViewportPanel.Height }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
			}

			ImGui::End();
		}
		ImGui::PopStyleVar();


		ImGui::End();
	}

	void SandboxLayer::OnEvent(StarFire::Event& e)
	{
		PROFILE_FUNCTION;

		StarFire::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<StarFire::FramebufferResizeEvent>(SF_BIND_EVENT_FN(SandboxLayer::OnFramebufferResize));
	}

	bool SandboxLayer::OnFramebufferResize(StarFire::FramebufferResizeEvent& e)
	{
		PROFILE_FUNCTION;

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
			return false;

		if (m_FramebufferWidth == e.GetWidth() && m_FramebufferHeight == e.GetHeight())
			return false;

		m_FramebufferWidth = e.GetWidth();
		m_FramebufferHeight = e.GetHeight();
		m_FramebufferResized = true;


		return false;
	}
}