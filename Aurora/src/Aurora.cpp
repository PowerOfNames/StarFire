#include "Aurora/Aurora.h"

#include "Core/Core.h"
#include "Core/RefRegistry.h"

namespace Aurora {

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

	bool InitializeRenderContext(const RenderContextSpecification& contextSpecs)
	{
		s_RenderContext = RenderContext::Create(contextSpecs);
		AURORA_ASSERT(s_RenderContext != nullptr, "Failed to create RenderContext.");			
		s_RenderContext->Init();
		return true;
	}

	bool Shutdown()
	{
		if (s_RenderContext)
			s_RenderContext->Shutdown();

		return true;
	}

}

