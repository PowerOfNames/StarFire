#pragma once

#include "StarFire/Core/KeyCodes.h"
#include "StarFire/Core/MouseCodes.h"

namespace StarFire {

	struct MousePosition
	{
		double X;
		double Y;
	};

	class Input
	{
	public:
		static bool IsKeyDown(KeyCode keyCode);
		static bool IsKeyUp(KeyCode keyCode);
		static bool IsMouseDown(MouseCode mouseCode);
		static bool IsMouseUp(MouseCode mouseCode);
		static MousePosition GetMousePos();
		static float GetMousePosX();
		static float GetMousePosY();
	};
}
