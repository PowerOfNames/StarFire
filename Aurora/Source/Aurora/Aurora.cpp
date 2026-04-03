#include "Aurora/Aurora.h"

#include "Aurora/Core/Core.h"
#include "Aurora/Core/RefRegistry.h"

#include "Aurora/Renderer/RenderContext.h"


namespace Aurora {
	namespace {
		Ref<VK::RenderContext> g_RenderContext = nullptr;
	}

	Ref<VK::RenderContext> GetRenderContext()
	{
		AURORA_ASSERT(g_RenderContext != nullptr, "RenderContext not initialized.");
		return g_RenderContext;
	}

	bool InitializeRenderContext(const RenderContextSpecification& contextSpecs)
	{
		if (g_RenderContext)
		{
			AURORA_WARN("RenderContext already initialized.");
			return false;
		}

		g_RenderContext = CreateRef<VK::RenderContext>(contextSpecs);
		AURORA_ASSERT(g_RenderContext != nullptr, "Failed to create RenderContext.");
		g_RenderContext->Init();
		return true;
	}

	bool BeginFrame()
	{
		return g_RenderContext->BeginFrame();
	}

	void EndFrame()
	{
		g_RenderContext->EndFrame();
	}

	void SwapFrame()
	{
		g_RenderContext->SwapFrame();
	}

	void Resize(uint32_t width, uint32_t height)
	{
		g_RenderContext->Resize(width, height);
	}

	bool Shutdown()
	{
		if (g_RenderContext)
			g_RenderContext->Destroy();

		return true;
	}

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
}