#include "sfpch.h"
#include "StarFire/Core/Input.h"

#include "StarFire/Core/Application.h"
#include "StarFire/Core/Core.h"

#include <GLFW/glfw3.h>

namespace StarFire {

	bool Input::IsKeyDown(KeyCode keyCode)
	{
		PROFILE_FUNCTION;

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetMainWindowPtr()->GetNativeWindow());
		int state = glfwGetKey(window, keyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsKeyUp(KeyCode keyCode)
	{
		return !IsKeyDown(keyCode);
	}

	bool Input::IsMouseDown(MouseCode mouseCode)
	{
		PROFILE_FUNCTION;

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetMainWindowPtr()->GetNativeWindow());
		int state = glfwGetMouseButton(window, mouseCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseUp(MouseCode mouseCode)
	{
		return !IsMouseDown(mouseCode);
	}

	MousePosition Input::GetMousePos()
	{
		PROFILE_FUNCTION;

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetMainWindowPtr()->GetNativeWindow());
		MousePosition pos{};
		glfwGetCursorPos(window, &(pos.X), &pos.Y);
		return pos;
	}

	float Input::GetMousePosX()
	{
		return static_cast<float>(GetMousePos().X);
	}

	float Input::GetMousePosY()
	{
		return static_cast<float>(GetMousePos().Y);
	}

	
}
