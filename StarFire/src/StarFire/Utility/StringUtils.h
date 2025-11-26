#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace StarFire::Utils::String {

	constexpr std::string ToLower(std::string_view in)
	{
		std::string result(in);
		std::ranges::transform(result, result.begin(), [](unsigned char c)
			{
				return std::tolower(c);
			});
		return result;
	}

	constexpr std::string ToLower(const std::string& in)
	{
		return ToLower(in);
	}
}
