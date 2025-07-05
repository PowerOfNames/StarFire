#pragma once

#include "Core/RefCounted.h"
#include "Renderer/VulkanSpecifics/VulkanCore.h"

namespace Aurora::VK {


	class VulkanShader : public RefCounted<VulkanShader>
	{
	public:
		VulkanShader() = default;
		~VulkanShader() = default;

		static constexpr const char* StaticTypeName() { return "VulkanShader"; }
	private:

	};
}
