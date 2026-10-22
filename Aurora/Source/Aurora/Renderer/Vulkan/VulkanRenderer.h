#pragma once
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/GraphicsPipeline.h"
#include "Aurora/Core/Core.h"

#include <functional>

namespace Aurora::VK {
	
	using RenderCommand = std::function<void(VkCommandBuffer cmd)>;

	struct BindlessSetData
	{
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	};


	class VulkanRenderer : public Substrate::RefCounted
	{
	public:
		VulkanRenderer() = default;
		~VulkanRenderer() = default;
		bool Init();
		void Destroy();

		void BindGlobalDescriptorSet(VkCommandBuffer cmd) const;
		void BindPipeline(VkCommandBuffer cmd, Ref<GraphicsPipeline> pipeline) const;
		void Draw(VkCommandBuffer cmd, uint32_t vertexCount);
		void DrawIndexed(VkCommandBuffer cmd, const IndexedDrawData& drawData);

		inline const BindlessSetData& GetBindlessSetData() const { return m_BindlessSetData; }

	private:
		bool CreateDefaultSamplers();
		bool CreateBindlessHandles();
		void WriteDefaultSamplers();

	private:
		BindlessSetData m_BindlessSetData{};

		std::vector<VkSampler> m_DefaultSamplers;

		inline static constexpr int SAMPLER_BINDING = 0;
		inline static constexpr int IMAGE_BINDING = 1;
	};
}