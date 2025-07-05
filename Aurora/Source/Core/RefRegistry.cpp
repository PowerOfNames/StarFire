#include "Core/RefRegistry.h"

namespace Aurora {


	void RefRegistry::SetRegisterCallback(ExternalRegistryRegisterCallback callback)
	{
		s_RegisterCallback = callback;
	}

	void RefRegistry::SetUnregisterCallback(ExternalRegistryUnregisterCallback callback)
	{
		s_UnregisterCallback = callback;
	}

	void RefRegistry::Register(const std::string& typeName, std::atomic<uint64_t>* counter)
	{
		if (s_RegisterCallback != nullptr)
			s_RegisterCallback(typeName, counter);
	}

	void RefRegistry::Unregister(const std::string& typeName)
	{
		if (s_UnregisterCallback != nullptr)
			s_UnregisterCallback(typeName);
	}

}
