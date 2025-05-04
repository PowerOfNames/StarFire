#pragma once
#include "StarFire/Core/Logging.h"
#include "StarFire/Memory/RefRegistry.h"

#include <atomic>
#include <memory>
#include <type_traits>

namespace StarFire {

	
	template<typename Derived>
	class RefCounted
	{
	public:
		~RefCounted()
		{
			s_RefCount.fetch_add(-1);
			SF_CORE_INFO("RefCount removed: {}", static_cast<uint64_t>(s_RefCount.load()));
			if (static_cast<uint64_t>(s_RefCount.load() == 0))
				RefRegistry::Get()->Unregister(typeid(Derived));
		}

		//template<typename T>
		//std::shared_ptr<T> As()
		//{
		//	return std::dynamic_pointer_cast<T>(shared_from_this());
		//}
		
	protected:
		RefCounted()
		{
			s_RefCount.fetch_add(1);
			SF_CORE_INFO("RefCount added: {}", static_cast<uint64_t>(s_RefCount.load()));
			if (static_cast<uint64_t>(s_RefCount.load() == 1))
				RefRegistry::Get()->Register(typeid(Derived), &s_RefCount);
		}
		RefCounted<Derived>(const RefCounted<Derived>&) = default;
		RefCounted<Derived>& operator=(const RefCounted<Derived>&) = default;

	private:
		inline static std::atomic<uint64_t> s_RefCount = 0;
	};
}
