#pragma once
#include "StarFire/Core/Assert.h"
#include "StarFire/Core/Core.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
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

		struct TransparentHash {
			using is_transparent = void; // marks this as transparent
			size_t operator()(std::string_view sv) const noexcept {
				return std::hash<std::string_view>{}(sv);
			}
			size_t operator()(const std::string& s) const noexcept {
				return std::hash<std::string_view>{}(s);
			}
		};

		struct TransparentEqual {
			using is_transparent = void; // marks this as transparent
			bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
				return lhs == rhs;
			}
			bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
				return lhs == rhs;
			}
			bool operator()(std::string_view lhs, const std::string& rhs) const noexcept {
				return lhs == rhs;
			}
			bool operator()(const std::string& lhs, std::string_view rhs) const noexcept {
				return lhs == rhs;
			}
		};

		std::unordered_map<std::string, std::atomic<uint64_t>*, TransparentHash, TransparentEqual> m_Registry;
		std::mutex m_RegistryMutex;
	};
}


