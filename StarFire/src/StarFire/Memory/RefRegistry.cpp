#include "sfpch.h"
#include "StarFire/Memory/RefRegistry.h"

#include "StarFire/Core/Core.h"
#include "StarFire/Core/Logging.h"



namespace StarFire {


	void RefRegistry::Register(const std::string& typeName, std::atomic<uint64_t>* counter)
	{
		std::lock_guard<std::mutex> lock(m_RegistryMutex);
		SF_CORE_ASSERT(m_Registry.find(typeName) == m_Registry.end(), "RefType already contained!");
		m_Registry[typeName] = counter;
	}


	void RefRegistry::Unregister(const std::string& typeName)
	{
		std::lock_guard<std::mutex> lock(m_RegistryMutex);
		SF_CORE_ASSERT(m_Registry.find(typeName) != m_Registry.end(), "RefType not contained contained!");
		m_Registry.erase(typeName);
	}


	void RefRegistry::PrintRegister()
	{
		std::lock_guard<std::mutex> lock(m_RegistryMutex);
		SF_CORE_INFO("Registry Entries: {}", m_Registry.size());
		for (const auto& entry : m_Registry)
		{
			SF_CORE_TRACE("Registry Entry '{}': Instances: {}", entry.first, static_cast<uint64_t>(entry.second->load()));
		}
	}

}
