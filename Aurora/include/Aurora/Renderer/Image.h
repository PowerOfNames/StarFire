#pragma once

#include "Aurora/Renderer/Types.h"

#include <cstdint>
#include <string>

namespace Aurora {

	struct ImageSpecification
	{
		std::string Name = "Image";
		uint32_t Width = 1;
		uint32_t Height = 1;

		Format Format = Format::UNKNOWN;
		ImageUsageFlags Usage = ImageUsageFlags::NONE;
		MemoryUsage MemUsage = MemoryUsage::GPU_ONLY;
		ImageTiling Tiling = ImageTiling::OPTIMAL;

		uint8_t MipLevels = 1;
		bool AllowResize = true;
	};
}
