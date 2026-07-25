#pragma once
#include "StarFire/Core/BitField.h"


namespace StarFire {
	enum class EventCategory : Substrate::BitField8
	{
		NONE = 0,
		APPLICATION = BIT(0),
		INPUT = BIT(1),
		KEYBOARD = BIT(2),
		MOUSE = BIT(3),
		MOUSE_BUTTON = BIT(4)
	};
}

namespace Substrate {
	SST_ENABLE_BIT_OPS(StarFire::EventCategory);
}