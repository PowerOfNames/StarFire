#include "Aurora/Aurora.h"

#include "Aurora/Core/Core.h"
#include "Aurora/Core/RefRegistry.h"

#include "Aurora/Renderer/RenderContext.h"

namespace {
	std::unique_ptr<Aurora::VK::RenderContext> s_RenderContext = nullptr;
}

namespace Aurora {

	bool InitializeRenderContext(const RenderContextSpecification& contextSpecs)
	{
		s_RenderContext = CreateScope<VK::RenderContext>(contextSpecs);
		AURORA_ASSERT(s_RenderContext != nullptr, "Failed to create RenderContext.");
		s_RenderContext->Init();
		return true;
	}

	bool BeginFrame()
	{
		return s_RenderContext->BeginFrame();
	}

	void EndFrame()
	{
		s_RenderContext->EndFrame();
	}

	void SwapFrame()
	{
		s_RenderContext->SwapFrame();
	}

	void Resize(uint32_t width, uint32_t height)
	{
		s_RenderContext->Resize(width, height);
	}

	bool Shutdown()
	{
		if (s_RenderContext)
			s_RenderContext->Destroy();

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