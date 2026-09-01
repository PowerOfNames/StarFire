#include "sfpch.h"

#include "StarFire/Core/Core.h"
#include "StarFire/ImGui/ImGuiLayer.h"
#include "StarFire/Core/Application.h"

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

		m_ImGuiRenderer = Aurora::ImGuiRenderer::Create();
	}

	void ImGuiLayer::OnAttach()
	{
		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Application::Get()->GetMainWindowPtr()->GetNativeWindow()), true);
		m_ImGuiRenderer->Init();
	}

	void ImGuiLayer::OnDetach()
	{
		m_ImGuiRenderer->Shutdown();
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

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<FramebufferResizeEvent>(SF_BIND_EVENT_FN(ImGuiLayer::OnFramebufferResize));
	}

	bool ImGuiLayer::OnFramebufferResize(FramebufferResizeEvent& e)
	{
		PROFILE_FUNCTION;

		uint32_t newWidth = e.GetWidth();
		uint32_t newHeight = e.GetHeight();
		if (newWidth == 0 || newHeight == 0)	
			return false;
		
		m_ImGuiRenderer->OnFramebufferResize(newWidth, newHeight);
		SF_CORE_DEBUG("New extent: [{}|{}]", newWidth, newHeight);

		return false;
	}

	void ImGuiLayer::BeginFrame() const
	{
		m_ImGuiRenderer->BeginFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::EndFrame() const
	{
		ImGui::Render();
		m_ImGuiRenderer->EndFrame();
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
