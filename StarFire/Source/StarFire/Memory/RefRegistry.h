#pragma once
#include "StarFire/Core/Core.h"

#include "StarFire/Core/StringKeyMap.h"

#include <atomic>
#include <memory>
#include <mutex>

namespace StarFire {

	class RefRegistry
	{
	public:
		RefRegistry(const RefRegistry&) = delete;
		RefRegistry& operator=(const RefRegistry&) = delete;
		RefRegistry(RefRegistry&&) = delete;
		RefRegistry& operator=(RefRegistry&&) = delete;
		
		~RefRegistry() = default;
				
		inline static RefRegistry* Get()
		{
			static RefRegistry instance;
			return &instance;
		}

		void Register(std::string_view typeName, std::atomic<uint64_t>* counter);
		void Unregister(std::string_view typeName);

		void PrintRegister();

	private:
		RefRegistry() = default;

	private:		
		Containers::StringKeyMap<std::atomic<uint64_t>*> m_Registry;
		std::mutex m_RegistryMutex;
	};
}


