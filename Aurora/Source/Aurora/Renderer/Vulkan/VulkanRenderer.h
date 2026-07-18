#pragma once
#include "Aurora/Renderer/Vulkan/VulkanCore.h"

#include "Aurora/Core/Core.h"

#include <deque>
#include <functional>

namespace Aurora::VK {
	
	using RenderCommand = std::function<void(VkCommandBuffer cmd)>;


	class VulkanRenderer : public Substrate::RefCounted
	{
	public:
		VulkanRenderer() = default;
		~VulkanRenderer() = default;
		bool Init();
		void Destroy();

		inline void SubmitRenderCommand(const RenderCommand&& cmd)
		{
			RenderCommandQueue.push_back(cmd);
		}

		// Delete if not used
		//inline const VkDescriptorPool GetBindlessDescriptorPool() const {
		//	return m_BindlessDescriptorPool;
		//}
		//inline const VkDescriptorSetLayout GetBindlessDescriptorSetLayout() const {
		//	return m_BindlessDescriptorSetLayout;
		//}
		//inline const VkDescriptorSet GetBindlessDescriptorSet() const {
		//	return m_BindlessDescriptorSet;
		//}

		//inline const VkPipeline GetBindlessGraphicsPipeline() const {
		//	return m_BindlessGraphicsPipeline;
		//}
		//inline const VkPipelineLayout GetBindlessGraphicsPipelineLayout() const {
		//	return m_BindlessGraphicsPipelineLayout;
		//}

	private:
		bool CreateBindlessDescriptorSet();
		bool CreateBindlessGraphicsPipeline();


	private:
		std::deque<RenderCommand> RenderCommandQueue;

		VkDescriptorPool m_BindlessDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_BindlessDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_BindlessDescriptorSet = VK_NULL_HANDLE;

		VkPipelineLayout m_BindlessGraphicsPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_BindlessGraphicsPipeline = VK_NULL_HANDLE;
	};
}