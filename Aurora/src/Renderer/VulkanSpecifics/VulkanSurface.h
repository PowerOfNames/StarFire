#pragma once
#include "Aurora/Renderer/Surface.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


namespace Aurora { namespace VK {

	class VulkanSurface : public Surface
	{
	public:
		VulkanSurface(const SurfaceSpecification& specs);
		~VulkanSurface() = default;

		virtual void Init() override;

		inline virtual const SurfaceSpecification& GetSpecifications() const override { return m_Specification; }

	private:
		SurfaceSpecification m_Specification{};

		GLFWwindow* m_WindowPtr = nullptr;
	};

}
}
