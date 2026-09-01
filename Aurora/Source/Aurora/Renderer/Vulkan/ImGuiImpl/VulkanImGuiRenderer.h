#pragma once
#include "Aurora/ImGui/ImGuiRenderer.h"
#include "Aurora/Renderer/Types.h"

#include "Aurora/Renderer/Vulkan/VulkanCore.h"

namespace Aurora::VK {


	class VulkanImGuiRenderer : public ImGuiRenderer
	{
	public:
		VulkanImGuiRenderer() = default;
		~VulkanImGuiRenderer() = default;

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void BeginFrame() override;
		virtual void OnFramebufferResize(uint32_t width, uint32_t height) override;
		virtual void EndFrame() override;
		virtual uint64_t GetTextureIDFromHandle(ImageHandle image) override;
		virtual void ReturnTextureIDFromHandle(ImageHandle image) override;

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		Format m_ImageFormat = Format::UNKNOWN;

		VkSampler m_TextureSampler = VK_NULL_HANDLE;

		std::unordered_map<ImageHandle, uint64_t> m_TextureIDMap; // Maps ImageHandles to ImGui texture IDs (which are actually VkDescriptorSet handles)

		std::vector<ImageHandle> m_RenderTargets;

		bool m_NeedsResize = false;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};

}