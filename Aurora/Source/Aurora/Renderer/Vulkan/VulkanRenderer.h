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
		
		bool RegisterTexture(ImageHandle textureHandle, VkImageLayout sampledLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void UnregisterTexture(ImageHandle textureHandle);
		uint32_t GetTextureSlot(ImageHandle textureHandle) const;

		inline const BindlessSetData& GetBindlessSetData() const { return m_BindlessSetData; }

	private:
		bool InitializeBindlessImageSlots();
		bool CreateFallbackTexture();
		bool CreateDefaultSamplers();
		bool CreateBindlessHandles();

	private:
		BindlessSetData m_BindlessSetData{};

		std::vector<VkSampler> m_DefaultSamplers;

		inline static constexpr uint32_t FALLBACK_IMAGE_SLOT_IDX = 0;
		inline static constexpr uint32_t MAX_IMAGE_REGISTER_COUNT = AURORA_MAX_IMAGE_ALLOCATIONS - 1;

		uint32_t m_FallbackCheckerBoardData[4] = { 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000 };
		ImageHandle m_FallbackTexture = ImageHandle::INVALID_HANDLE;

		std::unordered_map<ImageHandle, uint32_t> m_BindlessImageSlots;
		std::array<uint32_t, MAX_IMAGE_REGISTER_COUNT> m_BindlessImageSlotsFreeList;
		uint32_t m_FreeCount = MAX_IMAGE_REGISTER_COUNT;
	};
}