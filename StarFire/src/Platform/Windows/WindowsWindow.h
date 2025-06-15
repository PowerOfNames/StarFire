#pragma once
#include "StarFire/Core/Window.h"

#include <GLFW/glfw3.h>

namespace StarFire {
	namespace Platform {

		class WindowsWindow : public Window
		{
		public:
			WindowsWindow(const WindowSpecification& specs);

			virtual void Init() override;
			virtual void Close() override;

			virtual void OnUpdate() override;
			virtual void PollEvents() override;


			virtual inline uint32_t GetWidth() const override { return m_Specification.Width; }
			virtual inline uint32_t GetHeight() const override { return m_Specification.Height; }
			virtual inline uint32_t GetFramebufferWidth() const override { return m_Specification.FramebufferWidth; }
			virtual inline uint32_t GetFramebufferHeight() const override { return m_Specification.FramebufferHeight; }

			virtual const WindowSpecification& GetSpecification() const override { return m_Specification; }


			virtual inline void SetEventCallback(const EventCallbackFN& eventCallback) override { m_Data.EventCallback = eventCallback; };

			virtual void SetVSync(bool enabled) override;
			virtual bool IsVSync() const override { return m_Specification.VSync; }
			virtual void SetFullscreen(bool enable) override;
			virtual inline bool IsFullscreen() const override { return m_Specification.Fullscreen; }
			virtual void SetCursorState(CursorState state) override;
			virtual inline CursorState GetCursorState() const override { return m_Specification.MouseCursorState; };


			virtual inline void* GetNativeWindow() const override { return m_Window; }

		private:
			GLFWwindow* m_Window = nullptr;
			WindowSpecification m_Specification{};

			struct WindowData
			{
				std::string* Title = nullptr;
				int* Width = nullptr;
				int* Height = nullptr;

				uint32_t* FramebufferWidth = nullptr;
				uint32_t* FramebufferHeight = nullptr;

				float* ContentScaleX = nullptr;
				float* ContentScaleY = nullptr;

				EventCallbackFN EventCallback;
			};
			WindowData m_Data;
			bool m_GLFW_NO_API = false;
		};

	}
}

