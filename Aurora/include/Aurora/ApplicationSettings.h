#pragma once

#include <string>

namespace Aurora
{
	struct ApplicationSettings
	{
		std::string RootPath = "";
		std::string AssetPath = "";

		ApplicationSettings() {};
	};
}
