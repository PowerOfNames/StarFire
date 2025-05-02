#pragma once

#include "StarFire/Core/Timestep.h"
#include <spdlog/fmt/fmt.h> 


namespace fmt {

	template<>
	struct fmt::formatter<StarFire::Timestep> : fmt::formatter<std::string>
	{
		auto format(StarFire::Timestep my, format_context& ctx) const -> decltype(ctx.out())
		{
			return fmt::format_to(ctx.out(), "{}s", my.InSec());
		}
	};
}
