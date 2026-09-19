#pragma once

#include "StarFire/Core/Core.h"

#include <string_view>
#include <cstdint>
#include <charconv>
#include <optional>


namespace StarFire {


	class ApplicationArgumentParser
	{
	public:
		template<typename T>
		static std::optional<T> Parse(std::string_view token)
		{
			T result{};
			const char* end = token.data() + token.size();
			auto [ptr, ec] = std::from_chars(token.data(), end, result);
			if (ec == std::errc::invalid_argument)
			{
				STARFIRE_WARN("Argument token '{}' invalid argument", token);
				return std::nullopt;
			}
			if (ec == std::errc::result_out_of_range)
			{
				STARFIRE_WARN("Argument token '{}' out of range", token);
				return std::nullopt;
			}
			if (ptr != end)
			{
				STARFIRE_WARN("Argument token '{}' has trailing characters", token);
				return std::nullopt;
			}
			return result;
		}
	};


}