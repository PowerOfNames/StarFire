#include "SandboxLayer.h"
#include "Profiling/Profiling.h"

#include <imgui.h>

namespace Sandbox {

	SandboxLayer::SandboxLayer() : StarFire::Layer("SandboxLayer")
	{
		PROFILE_FUNCTION;

	}

	void SandboxLayer::OnAttach()
	{
		PROFILE_FUNCTION;

	}


	void SandboxLayer::OnDetach()
	{
		PROFILE_FUNCTION;

	}


	void SandboxLayer::OnUpdate(StarFire::Timestep deltaTime)
	{
		PROFILE_FUNCTION;

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

		if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
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
				if (ImGui::MenuItem("Nested Test", "Crt+O"))
					Test();

				ImGui::Separator();

				if (ImGui::MenuItem("Nested Test 2", "Crt+P"))
					Test();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::ShowDemoWindow();


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
