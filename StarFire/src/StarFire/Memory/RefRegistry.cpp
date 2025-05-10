#include "sfpch.h"
#include "StarFire/Memory/RefRegistry.h"

#include "StarFire/Core/Core.h"
#include "StarFire/Core/Logging.h"



namespace StarFire {


	void RefRegistry::Register(std::type_index type, std::atomic<uint64_t>* counter)
	{
		std::lock_guard<std::mutex> lock(m_RegistryMutex);
		SF_CORE_ASSERT(m_Registry.find(type) == m_Registry.end(), "RefType already contained!");
		m_Registry[type] = counter;
	}


	void RefRegistry::Unregister(std::type_index type)
	{
		std::lock_guard<std::mutex> lock(m_RegistryMutex);
		SF_CORE_ASSERT(m_Registry.find(type) != m_Registry.end(), "RefType not contained contained!");
		m_Registry.erase(type);
	}


	void RefRegistry::PrintRegister()
	{
		std::lock_guard<std::mutex> lock(m_RegistryMutex);
		SF_CORE_INFO("Registry Entries: {}", m_Registry.size());
		for (const auto& entry : m_Registry)
		{
			SF_CORE_TRACE("Registry Entry '{}': Instances: {}", entry.first.name(), static_cast<uint64_t>(entry.second->load()));
		}
	}

}
