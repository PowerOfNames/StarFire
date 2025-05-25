#include "Aurora/Aurora.h"

#include "Core/Logging.h"
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



		return true;
	}

	bool Shutdown()
	{
		return true;
	}

}

