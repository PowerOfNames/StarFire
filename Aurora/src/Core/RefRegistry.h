#pragma once

#include <atomic>
#include <string>
#include <typeindex>

namespace Aurora {

	class RefRegistry
	{
		using ExternalRegistryRegisterCallback = void(*)(const std::string& typeName, std::atomic<uint64_t>* counter);
		using ExternalRegistryUnregisterCallback = void(*)(const std::string& typeName);
		
	public:		

		static void SetRegisterCallback(ExternalRegistryRegisterCallback callback);
		static void SetUnregisterCallback(ExternalRegistryUnregisterCallback callback);
		static void Register(const std::string& typeName, std::atomic<uint64_t>* counter);
		static void Unregister(const std::string& typeName);

	private:
		inline static ExternalRegistryRegisterCallback s_RegisterCallback = nullptr;
		inline static ExternalRegistryUnregisterCallback s_UnregisterCallback = nullptr;
	};

}
