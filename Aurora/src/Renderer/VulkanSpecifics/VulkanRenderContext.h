#pragma once
#include "Aurora/Renderer/RenderContext.h"

#include "Renderer/VulkanSpecifics/VulkanCore.h"

namespace Aurora { namespace VK {

	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext(const RenderContextSpecification& specs);
		~VulkanRenderContext() = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		inline virtual const RenderContextSpecification& GetSpecification() const override { return m_Specification; }

	private:


	private:
		RenderContextSpecification m_Specification;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
	};

}
}