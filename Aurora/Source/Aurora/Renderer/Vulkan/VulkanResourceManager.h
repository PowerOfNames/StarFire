#pragma once

#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/Image.h"
#include "Aurora/Renderer/Buffer.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/VulkanContext.h"


#include "Substrate/PoolAllocator.h"

#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

namespace Aurora::VK {

	class VulkanResourceManager : public Substrate::RefCounted
	{
	public:
		VulkanResourceManager(const Ref<VulkanContext>& vulkanContext);
		~VulkanResourceManager() = default;

		void Destroy();

		ImageHandle CreateImage(const ImageSpecification& imageSpecs);
		bool IsHandleValid(ImageHandle handle);
		void DestroyImage(ImageHandle handle);
		VulkanImageData* GetImageData(ImageHandle handle);

		BufferHandle CreateBuffer(const BufferSpecification& bufferSpecs);
		bool IsHandleValid(BufferHandle handle);
		void DestroyBuffer(BufferHandle handle);
		VulkanBufferData* GetBufferData(BufferHandle handle);

		VertexBufferHandle CreateVertexBuffer(const VertexBufferSpecification& bufferSpecs);
		bool IsHandleValid(VertexBufferHandle handle);
		void DestroyVertexBuffer(VertexBufferHandle handle);
		VulkanBufferData* GetBufferData(VertexBufferHandle handle);

		IndexBufferHandle CreateIndexBuffer(const IndexBufferSpecification& bufferSpecs);
		bool IsHandleValid(IndexBufferHandle handle);
		void DestroyIndexBuffer(IndexBufferHandle handle);
		VulkanBufferData* GetBufferData(IndexBufferHandle handle);

	private:
		void UploadBufferData(BufferHandle handle, const void* data, size_t size);

	private:
		Ref<VulkanContext> m_VulkanContext = nullptr;
		bool m_ResourceManagerDestroyed = false;

		Substrate::PoolAllocator<VulkanImageData, ImageHandle::Type, 1024 * sizeof(VulkanImageData)> m_ImageAllocator; // we store up to 1024 images in the pool, which should be more than enough for now, but we can always add more pools with different sizes if needed or implement resize functionality
		Substrate::PoolAllocator<VulkanBufferData, BufferHandle::Type, 1024 * sizeof(VulkanBufferData)> m_BufferAllocator; // we store up to 1024 buffers in the pool, which should be more than enough for now, but we can always add more pools with different sizes if needed or implement resize functionality
	};

}
