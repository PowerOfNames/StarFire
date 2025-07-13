#pragma once
#include "StarFire/Core/Assert.h"
#include "StarFire/Core/Core.h"

#include "StarFire/Core/StringKeyMap.h"

#include <atomic>
#include <memory>
#include <mutex>

namespace StarFire {

	class Application;
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
			SF_CORE_ASSERT(s_Instance != nullptr, "Registry was not created yet! Application needs to call Init first!");
			return s_Instance.get(); 
		}

		void Register(std::string_view typeName, std::atomic<uint64_t>* counter);
		void Unregister(std::string_view typeName);

		void PrintRegister();

	private:
		RefRegistry() = default;		
		inline static void Init()
		{
			if (s_Instance == nullptr)
			{
				s_Instance = Scope<RefRegistry>(new RefRegistry);
			}
		}

	private:
		friend class StarFire::Application;

		inline static Scope<RefRegistry> s_Instance = nullptr;
		
		Containers::StringKeyMap<std::atomic<uint64_t>*> m_Registry;
		std::mutex m_RegistryMutex;
	};
}


