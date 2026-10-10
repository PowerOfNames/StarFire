#pragma once
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/GraphicsPipeline.h"
#include "Aurora/Core/Core.h"

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

		void BindPipeline(VkCommandBuffer cmd, Ref<GraphicsPipeline> pipeline);
		void Draw(VkCommandBuffer cmd, uint32_t vertexCount);
		void DrawIndexed(VkCommandBuffer cmd, const IndexedDrawData& drawData);
	};
}