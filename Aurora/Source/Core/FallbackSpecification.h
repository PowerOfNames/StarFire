#pragma once

#include "Renderer/APIType.h"

namespace Aurora {
	namespace FallbackSpecs {

		static constexpr APIType GetAPIType()
		{
			return APIType::API_TYPE_VULKAN;
		}
	}
}
