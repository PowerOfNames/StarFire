#include "sfpch.h"
#include "Platform/Windows/WindowsWindow.h"

#include "StarFire/Core/Assert.h"
#include "StarFire/Events/ApplicationEvent.h"
#include "StarFire/Events/KeyEvent.h"
#include "StarFire/Events/MouseEvent.h"

namespace StarFire{
	namespace Platform {

		static uint8_t s_GLFWwindowCount = 0;

		static void GLFWErrorCallback(int code, const char* description)
		{
			SF_CORE_ERROR("GLFW error ({}): {}", code, description);
		}

		WindowsWindow::WindowsWindow(const WindowSpecification& specs)
			: m_Specification(specs)
		{
		}

		void WindowsWindow::Close()
		{
			glfwDestroyWindow(m_Window);
			--s_GLFWwindowCount;
			glfwTerminate();
		}

		void WindowsWindow::Init()
		{
			m_Data.Title = &m_Specification.Title;
			m_Data.Width = (int*) & m_Specification.Width;
			m_Data.Height = (int*)&m_Specification.Height;

			if (s_GLFWwindowCount == 0)
			{
				int success = glfwInit();
				SF_CORE_ASSERT(success != 0, "Failed to initialize GLFW!");
				int major, minor, revision;
				glfwGetVersion(&major, &minor, &revision);
				SF_CORE_INFO("Initialized GLFW, compiled against version {}.{}.{}, running against version {}.{}.{}", 
					GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, 
					major, minor, revision);				
				glfwSetErrorCallback(GLFWErrorCallback);
			}
			//We only run with Vulkan!
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_GLFW_NO_API = true;

			m_Window = glfwCreateWindow(
				m_Specification.Width, 
				m_Specification.Height, 
				m_Specification.Title.c_str(), 
				m_Specification.Fullscreen ? glfwGetPrimaryMonitor() : NULL, 
				NULL);
			SF_CORE_ASSERT(m_Window != nullptr, "Failed to create GLFW Window");
			s_GLFWwindowCount++;
			SF_CORE_INFO("Created window number {}", s_GLFWwindowCount);
			


			glfwSetWindowUserPointer(m_Window, &m_Data);

			//Window event callbacks
			glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					Scope<Event> e = CreateScope<WindowCloseEvent>();
					data.EventCallback(std::move(e));			
				});

			//In screen coordinates
			glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					*data.Width = width;
					*data.Height = height;
					
					Scope<Event> e = CreateScope<WindowResizeEvent>(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
					data.EventCallback(std::move(e));
				});

			//In pixels
			glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

					Scope<Event> e = CreateScope<FramebufferResizeEvent>(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
					data.EventCallback(std::move(e));
				});

			//Key event callbacks
			glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

					switch (action)
					{
						case GLFW_PRESS:
						{
							Scope<Event> e = CreateScope<KeyPressedEvent>(key, 0);
							data.EventCallback(std::move(e));
							break;
						}
						case GLFW_RELEASE:
						{
							Scope<Event> e = CreateScope<KeyReleasedEvent>(key);
							data.EventCallback(std::move(e));
							break;
						}
						case GLFW_REPEAT:
						{
							Scope<Event> e = CreateScope<KeyPressedEvent>(key, 1);
							data.EventCallback(std::move(e));
							break;
						}
					}
				});

			glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					
					Scope<Event> e = CreateScope<KeyTypedEvent>(keycode);
					data.EventCallback(std::move(e));
				});

			//Mouse event callbacks
			//in screen coordinates, relative to top left corned of window content area
			glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

					Scope<Event> e = CreateScope<MouseMoveEvent>(static_cast<float>(xPos), static_cast<float>(yPos));
					data.EventCallback(std::move(e));
				});

			glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					
					switch (action)
					{
						case GLFW_PRESS:
						{
							Scope<Event> e = CreateScope<MousePressEvent>(button, 0);
							data.EventCallback(std::move(e));
							break;
						}
						case GLFW_RELEASE:
						{
							Scope<Event> e = CreateScope<MouseReleasedEvent>(button);
							data.EventCallback(std::move(e));
							break;
						}
						case GLFW_REPEAT:
						{
							Scope<Event> e = CreateScope<MousePressEvent>(button, 1);
							data.EventCallback(std::move(e));
							break;
						}
					}
				});

			glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
				{
					WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
					
					Scope<Event> e = CreateScope<MouseScrolledEvent>(static_cast<float>(xOffset), static_cast<float>(yOffset));
					data.EventCallback(std::move(e));
				});

			//TODO: Joystick
			//TODO: Clipboard IO
			//TODO: File/directory dropping
		}

		void WindowsWindow::OnUpdate()
		{
			//glfwSwapBuffers(m_Window);
		}


		void WindowsWindow::PollEvents()
		{
			glfwPollEvents();
		}


		void WindowsWindow::SetVSync(bool enabled)
		{
			if (enabled)
				glfwSwapInterval(0);
			else
				glfwSwapInterval(1);
			m_Specification.VSync = enabled;
		}


		void WindowsWindow::SetFullscreen(bool enabled)
		{
			if (m_Window == nullptr)
			{
				SF_CORE_ERROR("No window pointer set!");
				return;
			}

			if (enabled)
			{
				GLFWmonitor* monitor = glfwGetWindowMonitor(m_Window);
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			}
			else
				glfwSetWindowMonitor(
					m_Window, 
					NULL, 
					m_Specification.PositionX,
					m_Specification.PositionY, 
					m_Specification.Width,
					m_Specification.Height,
					0
				);
			m_Specification.Fullscreen = enabled;
		}

		void WindowsWindow::SetCursorState(CursorState state)
		{
			switch (state)
			{
				case CursorState::NORMAL:
				{
					glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					break;
				}
				case CursorState::DISABLED:
				{
					glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					if (glfwRawMouseMotionSupported())
						glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
					break;
				}
				case CursorState::HIDDEN:
				{
					glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					break;
				}
				case CursorState::CAPTURED:
				{
					glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
					break;
				}
				default:
				{
					SF_CORE_WARN("Cursor state not found. Fallback to normal");
					glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}

			if(state != CursorState::DISABLED)
				glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			m_Specification.MouseCursorState = state;
		}

	}
}
