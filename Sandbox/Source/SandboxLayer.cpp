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
		rgSpecs.Name = "Default Render Graph";
		m_DefaultRenderGraph = Aurora::RenderGraph::Create(rgSpecs);

		uint32_t width = 800;
		uint32_t height = 600;
		// == Simple 2D pass for triangle rendering as test ==
		{
			Aurora::RenderPassSpecification specs{};
			specs.Name = "Triangle Render Pass";
			Aurora::ColorAttachmentSpecification colorAttachment{};
			colorAttachment.ImageSpecs.Name = "Color Attachment";
			colorAttachment.ImageSpecs.Usage = Aurora::ImageUsageFlags::COLOR_ATTACHMENT | Aurora::ImageUsageFlags::SAMPLED | Aurora::ImageUsageFlags::TRANSFER_SRC;
			colorAttachment.ImageSpecs.Format = Aurora::Format::RGBA8_UNORM;
			colorAttachment.ImageSpecs.Width = width;
			colorAttachment.ImageSpecs.Height = height;
			colorAttachment.ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
			specs.ColorAttachments.push_back(std::move(colorAttachment));

			specs.DepthAttachment.Name = "Depth Attachment";
			specs.DepthAttachment.ImageSpecs.Usage = Aurora::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT | Aurora::ImageUsageFlags::TRANSFER_SRC;
			specs.DepthAttachment.ImageSpecs.Format = Aurora::Format::DEPTH32_SFLOAT;
			specs.DepthAttachment.ImageSpecs.Width = width;
			specs.DepthAttachment.ImageSpecs.Height = height;
			specs.DepthAttachment.ClearDepth = 1.0f; //needs to be 0.0 if we do infinite far plane, but for now we do a finite far plane so 1.0 is correct
			specs.RenderArea = { width, height };	
			m_TrianglePass = Aurora::RenderPass::Create(specs);
		}

		{
			Aurora::ImageSpecification viewportTarget{};
			viewportTarget.Name = "Viewport Target";
			viewportTarget.Usage = Aurora::ImageUsageFlags::SAMPLED | Aurora::ImageUsageFlags::TRANSFER_DST;
			viewportTarget.Format = Aurora::Format::RGBA8_UNORM;
			viewportTarget.Width = width;
			viewportTarget.Height = height;
			//TODO: needs verificatrion. I think there was one place in the current pipeline that demanded linear tiling, but I forgot where it was.
			//viewportTarget.Tiling = Aurora::ImageTiling::LINEAR;
			m_ViewportImageHandle = Aurora::CreateImage(viewportTarget);
		}

		//m_ViewportTextureID = StarFire::Application::Get()->GetImGuiLayer()->GetImGuiRenderer()->GetTextureIDFromHandle(m_TrianglePass->GetColorAttachmentHandle());


		m_DefaultRenderGraph->AddRenderPass(m_TrianglePass);
		//Aurora::ImageCopyInfo cpyInfo{};
		//cpyInfo.SrcImage = m_TrianglePass->GetColorAttachmentHandle();
		//cpyInfo.DstImage = m_ViewportImageHandle;
		//m_DefaultRenderGraph->AddImageCopy(cpyInfo);

		//This compiles the architecture given during creation and sets up everything. This should contain the complete capability of this render graph
		m_DefaultRenderGraph->Compile();
	}

	void SandboxLayer::OnAttach()
	{
		PROFILE_FUNCTION;

		// == 1. Build scene ==
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

		StarFire::Application::Get()->GetImGuiLayer()->GetImGuiRenderer()->ReturnTextureIDFromHandle(m_ViewportImageHandle);
		Aurora::DestroyImage(m_ViewportImageHandle);

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
			//TODO: Use the viewport's focused/hovered state to control whether the camera controller should receive input, etc.

			ImGui::Image(m_ViewportTextureID, ImVec2{ m_ViewportPanel.Bounds[1].x - m_ViewportPanel.Bounds[0].x, m_ViewportPanel.Bounds[1].y - m_ViewportPanel.Bounds[0].y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			ImGui::End();
		}
		ImGui::PopStyleVar();


		ImGui::End();
	}

	void SandboxLayer::OnEvent(StarFire::Event& e)
	{
		PROFILE_FUNCTION;

		StarFire::EventDispatcher dispatcher(e);
	}



	void SandboxLayer::Test()
	{
		PROFILE_FUNCTION;


	}
}
