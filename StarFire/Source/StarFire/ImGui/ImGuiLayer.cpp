#include "sfpch.h"

#include "StarFire/ImGui/ImGuiLayer.h"
#include "StarFire/Core/Application.h"
#include "Aurora/ImGui/ImGuiRenderer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#include <GLFW/glfw3.h>

namespace StarFire {

	ImGuiLayer::ImGuiLayer()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard 
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable multiple viewports

		io.ConfigDpiScaleFonts = true; // Enable DPI scaling for fonts - automatically scales the font size based on DPpi of current display

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

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategory::MOUSE) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategory::KEYBOARD) & io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::BeginFrame() const
	{
		Aurora::ImGuiImpl::BeginFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::EndFrame() const
	{
		ImGui::Render();
		Aurora::ImGuiImpl::EndFrame();
		ImGui::EndFrame();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}
