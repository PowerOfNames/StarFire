#include "sfpch.h"

#include "StarFire/ImGui/ImGuiLayer.h"
#include "StarFire/Core/Application.h"
#include "Aurora/ImGui/ImGuiRenderer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>


namespace StarFire {

	ImGuiLayer::ImGuiLayer()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

		ImGui::StyleColorsDark();
	}

	void ImGuiLayer::OnAttach()
	{
		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Application::Get()->GetMainWindowPtr()->GetNativeWindow()), true);
		Aurora::ImGuiImpl::Init();
	}

	void ImGuiLayer::OnDetach()
	{
		Aurora::ImGuiImpl::Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::PreUpdate(Timestep ts)
	{
		Aurora::ImGuiImpl::BeginFrame();
		ImGui_ImplGlfw_NewFrame();
	}


	void ImGuiLayer::PostUpdate(Timestep ts)
	{
		ImGui::Render();
		Aurora::ImGuiImpl::EndFrame();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategory::MOUSE) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategory::KEYBOARD) & io.WantCaptureKeyboard;
		}
	}
}
