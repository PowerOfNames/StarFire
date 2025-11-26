#pragma once
#include "Aurora/Assets/ShaderDescriptions.h"

#include <string>
#include <vector>

namespace Aurora {

	struct Material
	{
		std::string Name;
		std::vector<DescriptorBinding> Parameters;
	};
}
