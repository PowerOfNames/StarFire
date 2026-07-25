#include "Aurora/Renderer/Vulkan/VulkanResourceManager.h"

#include "Aurora/Core/Logging.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanCreators.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"

namespace Aurora::VK {
	VulkanResourceManager::VulkanResourceManager(const Ref<VulkanContext>& vulkanContext)
		: m_VulkanContext(vulkanContext)
	{
		PROFILE_FUNCTION;

		if (!m_VulkanContext)
		{
			AURORA_ERROR("VulkanContext cannot be nullptr");
			return;
		}
	}

	void VulkanResourceManager::Destroy()
	{
		PROFILE_FUNCTION;

		m_ImageAllocator.Reset();
		m_BufferAllocator.Reset();		
	}

	void VulkanResourceManager::UploadBufferData(BufferHandle handle, const void* data, size_t size)
	{
		PROFILE_FUNCTION;


		if (handle == BufferHandle::INVALID_HANDLE || !data || size == 0)
		{
			AURORA_ERROR("Invalid parameters for UploadBufferData. BufferData pointer: {}, Data pointer: {}, Size: {}.", (uint64_t)handle, (void*)data, size);
			return;
		}

		BufferHandle stagingHandle = m_BufferAllocator.Allocate();
		if (stagingHandle == BufferHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("Failed to allocate staging buffer handle. Maximum number of buffers reached.");
			return;
		}

		VulkanBufferData* stagingData = m_BufferAllocator.GetPointerFromHandle(stagingHandle);
		stagingData->Size = size;
		stagingData->Offset = 0;
		bool success = Creators::CreateBuffer(m_VulkanContext->GetVmaAllocator(), &(stagingData->Buffer), &(stagingData->Allocation), &(stagingData->AllocationInfo), Convert::ToVkBufferUsageFlags(BufferUsageFlags::TRANSFER_SRC), VMA_MEMORY_USAGE_CPU_TO_GPU, size);
		if (!success)
		{
			AURORA_ERROR("Failed to create staging buffer for handle {}. Freeing handle.", static_cast<uint16_t>(stagingHandle));
			m_BufferAllocator.Free(stagingHandle);
			return;
		}
		AURORA_VK_ATTACH_DEBUG_NAME(m_VulkanContext->GetLogicalDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)(stagingData->Buffer), "StagingBuffer");

		//Queue family owner is initially unknown. It will be owned by the first user, which will bei either transfer or graphics (decicion is taken
		// later during the actual upload call based on differen metrics)
		stagingData->LastOwner = QueueOwner::UNKNOWN;
		stagingData->CurrentOwner = QueueOwner::UNKNOWN;
		stagingData->NextOwner = QueueOwner::UNKNOWN;
		void* mappedData = nullptr;
		vmaMapMemory(m_VulkanContext->GetVmaAllocator(), stagingData->Allocation, &mappedData);
		if (!mappedData)
		{
			AURORA_ERROR("Failed to map memory for buffer upload.");
			vmaDestroyBuffer(m_VulkanContext->GetVmaAllocator(), stagingData->Buffer, stagingData->Allocation);
			m_BufferAllocator.Free(stagingHandle);
			return;
		}
		std::memcpy(mappedData, data, size);
		vmaUnmapMemory(m_VulkanContext->GetVmaAllocator(), stagingData->Allocation);

		m_VulkanContext->CopyBufferToBuffer(stagingHandle, handle, false, true);
	}


	// ========== Image ==========
	ImageHandle VulkanResourceManager::CreateImage(const ImageSpecification& imageSpecs)
	{
		PROFILE_FUNCTION;
			
		if (imageSpecs.Width == 0 || imageSpecs.Height == 0)
		{
			AURORA_ERROR("RendererMemoryManager::CreateImage: Image width and height cannot be 0. Returning invalid handle.");
			return ImageHandle::INVALID_HANDLE;
		}

		if (imageSpecs.MipLevels == 0)
		{
			AURORA_ERROR("RendererMemoryManager::CreateImage: MipLevels cannot be 0. Freeing handle and returning invalid handle.");
			return ImageHandle::INVALID_HANDLE;
		}

		ImageHandle handle = m_ImageAllocator.Allocate();
		if (handle == ImageHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("RendererMemoryManager::CreateImage: Failed to allocate image handle. Maximum number of images reached.");
			return ImageHandle::INVALID_HANDLE;
		}		

		VulkanImageData* data = m_ImageAllocator.GetPointerFromHandle(handle);
		*data = Convert::MakeImageData(imageSpecs);
		// TODO: imageSpecs.MemUsage is dropped here; GPU_ONLY matches the value
		//       Creators::CreateImage used to hardcode internally.
		bool success = Creators::CreateImage(m_VulkanContext->GetVmaAllocator(), *data, VMA_MEMORY_USAGE_GPU_ONLY);
		if (!success)
		{
			AURORA_ERROR("RendererMemoryManager.CreateImage: Failed to create image for handle {}. Freeing handle.", static_cast<uint16_t>(handle));
			m_ImageAllocator.Free(handle);
			return ImageHandle::INVALID_HANDLE;
		}
		AURORA_VK_ATTACH_DEBUG_NAME(m_VulkanContext->GetLogicalDevice(), VK_OBJECT_TYPE_IMAGE, (uint64_t)(data->Image), imageSpecs.Name.c_str());

		success &= Creators::CreateImageView(m_VulkanContext->GetLogicalDevice(), m_VulkanContext->GetAllocationCallbacks(), *data);

		if (!success)
		{
			AURORA_ERROR("RendererMemoryManager.CreateImage: Failed to create image or image view for handle {}. Freeing handle.", static_cast<uint16_t>(handle));
			m_ImageAllocator.Free(handle);
			return ImageHandle::INVALID_HANDLE;
		}
		std::string imageViewName = imageSpecs.Name + "_ImageView";
		AURORA_VK_ATTACH_DEBUG_NAME(m_VulkanContext->GetLogicalDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)(data->ImageView), imageViewName.c_str());

		return handle;
	}

	void VulkanResourceManager::DestroyImage(ImageHandle handle)
	{
		PROFILE_FUNCTION;

		VulkanImageData* data = m_ImageAllocator.GetPointerFromHandle(handle);
		if (!data)
		{
			AURORA_ERROR("Failed to retrieve image data pointer during destruction. Leaking memory might happen");
			return;
		}
		vkDestroyImageView(m_VulkanContext->GetLogicalDevice(), data->ImageView, m_VulkanContext->GetAllocationCallbacks());
		vmaDestroyImage(m_VulkanContext->GetVmaAllocator(), data->Image, data->Allocation);

		m_ImageAllocator.Free(handle);
	}

	bool VulkanResourceManager::IsHandleValid(ImageHandle handle)
	{
		PROFILE_FUNCTION;

		return m_ImageAllocator.IsHandleValid(handle);
	}

	VulkanImageData* VulkanResourceManager::GetImageData(ImageHandle handle)
	{
		PROFILE_FUNCTION;

		return m_ImageAllocator.GetPointerFromHandle(handle);
	}
	
	// ========== Buffer ==========
	BufferHandle VulkanResourceManager::CreateBuffer(const BufferSpecification& bufferSpecs)
	{
		PROFILE_FUNCTION;

		if (bufferSpecs.Size == 0)
		{
			AURORA_ERROR("Buffer size cannot be 0.");
			return BufferHandle::INVALID_HANDLE;
		}

		BufferHandle handle = m_BufferAllocator.Allocate();
		if (handle == BufferHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("Failed to allocate buffer handle. Maximum number of buffers reached.");
			return BufferHandle::INVALID_HANDLE;
		}

		VulkanBufferData* data = m_BufferAllocator.GetPointerFromHandle(handle);
		data->Size = bufferSpecs.Size;
		bool success = Creators::CreateBuffer(m_VulkanContext->GetVmaAllocator(), &(data->Buffer), &(data->Allocation), &(data->AllocationInfo), Convert::ToVkBufferUsageFlags(bufferSpecs.Usage), VMA_MEMORY_USAGE_GPU_ONLY, bufferSpecs.Size);

		if (!success)
		{
			AURORA_ERROR("Failed to create buffer for handle {}. Freeing handle.", static_cast<uint16_t>(handle));
			m_BufferAllocator.Free(handle);
			return BufferHandle::INVALID_HANDLE;
		}
		AURORA_VK_ATTACH_DEBUG_NAME(m_VulkanContext->GetLogicalDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)(data->Buffer), bufferSpecs.Name.c_str());

		return handle;
	}

	void VulkanResourceManager::DestroyBuffer(BufferHandle handle)
	{
		PROFILE_FUNCTION;

		VulkanBufferData* data = m_BufferAllocator.GetPointerFromHandle(handle);
		if (!data)
		{
			AURORA_ERROR("Failed to retrieve buffer data pointer during destruction. Leaking memory might happen");
			return;
		}

		// CAUTION: this currently only works for Submission only resources. As soon as a resource is used by the GPU during frames, which do currently not use the submission semaphores (queuSemaphors)
		//			this is not true anymore.
		TimelineSemaphore semaphoreSnapshot = m_VulkanContext->GetQueueSemaphoreSnapshot(data->CurrentOwner);
		m_VulkanContext->SubmitToFrameDeletionQueue(
			[
				vmaAllocator = m_VulkanContext->GetVmaAllocator(), 
				buffer = data->Buffer, 
				allocation = data->Allocation
			]()
		{
			vmaDestroyBuffer(vmaAllocator, buffer, allocation);
		}, semaphoreSnapshot.Semaphore, semaphoreSnapshot.Value);

		m_BufferAllocator.Free(handle);
	}

	bool VulkanResourceManager::IsHandleValid(BufferHandle handle)
	{
		PROFILE_FUNCTION;

		return m_BufferAllocator.IsHandleValid(handle);
	}

	VulkanBufferData* VulkanResourceManager::GetBufferData(BufferHandle handle)
	{
		PROFILE_FUNCTION;
		return m_BufferAllocator.GetPointerFromHandle(handle);
	}

	// == Vertex Buffer ==
	VertexBufferHandle VulkanResourceManager::CreateVertexBuffer(const VertexBufferSpecification& bufferSpecs)
	{
		PROFILE_FUNCTION;

		if (bufferSpecs.Size == 0)
		{
			AURORA_ERROR("Buffer size cannot be 0.");
			return BufferHandle::INVALID_HANDLE;
		}

		VertexBufferHandle handle = m_BufferAllocator.Allocate();
		if (handle == VertexBufferHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("Failed to allocate buffer handle. Maximum number of buffers reached.");
			return VertexBufferHandle::INVALID_HANDLE;
		}

		VulkanBufferData* data = GetBufferData(handle);
		data->Size = bufferSpecs.Size;
		data->Offset = 0;
		data->Usage = bufferSpecs.Usage | BufferUsageFlags::TRANSFER_DST | BufferUsageFlags::SHADER_DEVICE_ADDRESS;
		if (!Creators::CreateBuffer(m_VulkanContext->GetVmaAllocator(), &(data->Buffer), &(data->Allocation), &(data->AllocationInfo), Convert::ToVkBufferUsageFlags(data->Usage), Convert::ToVmaMemoryUsage(bufferSpecs.MemUsage), data->Size))
		{
			AURORA_ERROR("Failed to create static vertex buffer for bindless rendering. Bindless rendering might not work correctly.");
			m_BufferAllocator.Free(handle);
			return VertexBufferHandle::INVALID_HANDLE;
		}
		AURORA_VK_ATTACH_DEBUG_NAME(m_VulkanContext->GetLogicalDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)(data->Buffer), "StaticVertexBuffer");
		data->LastOwner = QueueOwner::UNKNOWN;
		data->CurrentOwner = QueueOwner::UNKNOWN;
		data->NextOwner = QueueOwner::UNKNOWN;

		// if bufferSpecs.Data is not null, we need to transfer the buffer at offset with size from current ownershitp to transfer queue 
		// and then upload the data via a staging buffer and then copy the staging content into data
		if (bufferSpecs.Data)
		{
			data->IsReady = false; // mark buffer as not ready until the upload is finished
			UploadBufferData(handle.As<BufferHandle>(), bufferSpecs.Data, data->Size);
		}
		

		return handle;
	}

	void VulkanResourceManager::DestroyVertexBuffer(VertexBufferHandle handle)
	{
		PROFILE_FUNCTION;

		VulkanBufferData* data = m_BufferAllocator.GetPointerFromHandle(handle);
		if (!data)
		{
			AURORA_ERROR("Failed to retrieve buffer data pointer during destruction. Leaking memory might happen");
			return;
		}
		vmaDestroyBuffer(m_VulkanContext->GetVmaAllocator(), data->Buffer, data->Allocation);

		m_BufferAllocator.Free(handle);
	}

	bool VulkanResourceManager::IsHandleValid(VertexBufferHandle handle)
	{
		PROFILE_FUNCTION;

		return m_BufferAllocator.IsHandleValid(handle);
	}

	VulkanBufferData* VulkanResourceManager::GetBufferData(VertexBufferHandle handle)
	{
		PROFILE_FUNCTION;
		return m_BufferAllocator.GetPointerFromHandle(handle);
	}

	// == Index Buffer ==
	IndexBufferHandle VulkanResourceManager::CreateIndexBuffer(const IndexBufferSpecification& bufferSpecs)
	{
		PROFILE_FUNCTION;

		if (bufferSpecs.Size == 0)
		{
			AURORA_ERROR("Buffer size cannot be 0.");
			return IndexBufferHandle::INVALID_HANDLE;
		}

		IndexBufferHandle handle = m_BufferAllocator.Allocate();
		if (handle == IndexBufferHandle::INVALID_HANDLE)
		{
			AURORA_ERROR("Failed to allocate buffer handle. Maximum number of buffers reached.");
			return IndexBufferHandle::INVALID_HANDLE;
		}

		VulkanBufferData* data = m_BufferAllocator.GetPointerFromHandle(handle);
		data->Size = bufferSpecs.Size;
		bool success = Creators::CreateBuffer(m_VulkanContext->GetVmaAllocator(), &(data->Buffer), &(data->Allocation), &(data->AllocationInfo), Convert::ToVkBufferUsageFlags(bufferSpecs.Usage), VMA_MEMORY_USAGE_GPU_ONLY, bufferSpecs.Size);

		if (!success)
		{
			AURORA_ERROR("Failed to create buffer for handle {}. Freeing handle.", static_cast<uint16_t>(handle));
			m_BufferAllocator.Free(handle);
			return IndexBufferHandle::INVALID_HANDLE;
		}
		AURORA_VK_ATTACH_DEBUG_NAME(m_VulkanContext->GetLogicalDevice(), VK_OBJECT_TYPE_BUFFER, (uint64_t)(data->Buffer), bufferSpecs.Name.c_str());

		return handle;
	}

	void VulkanResourceManager::DestroyIndexBuffer(IndexBufferHandle handle)
	{
		PROFILE_FUNCTION;

		VulkanBufferData* data = m_BufferAllocator.GetPointerFromHandle(handle);
		if (!data)
		{
			AURORA_ERROR("Failed to retrieve buffer data pointer during destruction. Leaking memory might happen");
			return;
		}
		vmaDestroyBuffer(m_VulkanContext->GetVmaAllocator(), data->Buffer, data->Allocation);

		m_BufferAllocator.Free(handle);
	}

	bool VulkanResourceManager::IsHandleValid(IndexBufferHandle handle)
	{
		PROFILE_FUNCTION;

		return m_BufferAllocator.IsHandleValid(handle);
	}

	VulkanBufferData* VulkanResourceManager::GetBufferData(IndexBufferHandle handle)
	{
		PROFILE_FUNCTION;
		return m_BufferAllocator.GetPointerFromHandle(handle);
	}
}