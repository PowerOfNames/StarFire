#pragma once
#include "StarFire/Core/Assert.h"
#include "StarFire/Core/Core.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>

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

		void Register(std::type_index index, std::atomic<uint64_t>* counter);
		void Unregister(std::type_index index);

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
		std::unordered_map<std::type_index, std::atomic<uint64_t>*> m_Registry;
		std::mutex m_RegistryMutex;
	};
}


