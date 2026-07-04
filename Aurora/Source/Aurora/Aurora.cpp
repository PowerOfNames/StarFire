#include "Aurora/Aurora.h"

#include "Aurora/Core/Core.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Core/RefRegistry.h"

#include "Aurora/Renderer/Vulkan/VulkanContext.h"
#include "Aurora/Renderer/Vulkan/VulkanResourceManager.h"

namespace Aurora {
	namespace {
		Ref<VK::VulkanContext> g_RenderContext = nullptr;
		Ref<VK::VulkanResourceManager> g_ResourceManager = nullptr;
	}
	Ref<VK::VulkanContext> GetRenderContext()
	{
		AURORA_ASSERT(g_RenderContext != nullptr, "RenderContext not initialized.");
		return g_RenderContext;
	}

	Ref<VK::VulkanResourceManager> GetResourceManager()
	{
		AURORA_ASSERT(g_ResourceManager != nullptr, "ResourceManager not initialized.");
		return g_ResourceManager;
	}

	// ========== Setup ==========
	void SetLoggingCallback(LogCallback callback)
	{
		Log::SetCallback(callback);
	}

	void SetRefRegistryRegisterCallback(RegistryRegisterCallback callback)
	{
		RefRegistry::SetRegisterCallback(callback);
	}

	void SetRefRegistryUnregisterCallback(RegistryUnregisterCallback callback)
	{
		RefRegistry::SetUnregisterCallback(callback);
	}

	bool Initialize(const InitializationSpecification& contextSpecs)
	{
		PROFILE_FUNCTION;

		if (g_RenderContext)
		{
			AURORA_WARN("RenderContext already initialized.");
			return false;
		}

		g_RenderContext = CreateRef<VK::VulkanContext>(contextSpecs);
		AURORA_ASSERT(g_RenderContext != nullptr, "Failed to create RenderContext.");
		g_RenderContext->Init();
		g_ResourceManager = CreateRef<VK::VulkanResourceManager>(g_RenderContext);
		return true;
	}

	// ========== Context ==========
	bool BeginFrame()
	{
		PROFILE_FUNCTION;

		return g_RenderContext->BeginFrame();
	}

	bool Shutdown()
	{
		PROFILE_FUNCTION;

		if (g_RenderContext)
			g_RenderContext->Destroy();

		return true;
	}

	void EndFrame()
	{
		PROFILE_FUNCTION;
		
		g_RenderContext->EndFrame();
	}

	void SwapFrame()
	{
		PROFILE_FUNCTION;

		g_RenderContext->SwapFrame();
	}

	void Resize(uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION;

		g_RenderContext->Resize(width, height);
	}

	// ========== Resources ==========
	// ===== Image =====
	ImageHandle CreateImage(const ImageSpecification& imageSpecs)
	{
		PROFILE_FUNCTION;

		return g_ResourceManager->CreateImage(imageSpecs);
	}

	bool IsHandleValid(ImageHandle handle)
	{
		PROFILE_FUNCTION;

		return g_ResourceManager->IsHandleValid(handle);
	}

	void DestroyImage(ImageHandle handle)
	{
		PROFILE_FUNCTION;

		g_ResourceManager->DestroyImage(handle);
	}

	// ===== Buffer =====
	BufferHandle CreateBuffer(const BufferSpecification& bufferSpecs)
	{
		PROFILE_FUNCTION;

		return g_ResourceManager->CreateBuffer(bufferSpecs);
	}

	void DestroyBuffer(BufferHandle handle)
	{
		PROFILE_FUNCTION;

		g_ResourceManager->DestroyBuffer(handle);
	}

	VertexBufferHandle CreateVertexBuffer(const VertexBufferSpecification& bufferSpecs)
	{
		PROFILE_FUNCTION;

		return g_ResourceManager->CreateVertexBuffer(bufferSpecs);
	}

	void DestroyVertexBuffer(VertexBufferHandle handle)
	{
		PROFILE_FUNCTION;

		g_ResourceManager->DestroyVertexBuffer(handle);
	}

	IndexBufferHandle CreateIndexBuffer(const IndexBufferSpecification& bufferSpecs)
	{
		PROFILE_FUNCTION;

		return g_ResourceManager->CreateIndexBuffer(bufferSpecs);
	}

	void DestroyIndexBuffer(IndexBufferHandle handle)
	{
		PROFILE_FUNCTION;

		g_ResourceManager->DestroyIndexBuffer(handle);
	}

}