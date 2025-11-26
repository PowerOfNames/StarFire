#pragma once

#include "Aurora/Core/RefRegistry.h"

#include <atomic>
#include <memory>
#include <type_traits>
#include <concepts>

namespace Aurora {

	template<typename T>
	class RefCounted : public std::enable_shared_from_this<T>
	{
	public:
		~RefCounted()
		{			
			if (s_RefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				RefRegistry::Unregister(T::StaticTypeName());
		}

		template<typename Derived>
		std::enable_if_t<std::is_base_of_v<T, Derived>, std::shared_ptr<Derived>> As()
		{
			return std::dynamic_pointer_cast<Derived>(this->shared_from_this());
		}

		std::shared_ptr<T> GetPtr()
		{
			return this->shared_from_this();
		}

	protected:
		RefCounted()
		{			
			if (s_RefCount.fetch_add(1, std::memory_order_acq_rel) == 0)
				RefRegistry::Register(T::StaticTypeName(), &s_RefCount);
		}
		RefCounted<T>(const RefCounted<T>&) = default;
		RefCounted<T>& operator=(const RefCounted<T>&) = default;

	private:
		inline static std::atomic<uint64_t> s_RefCount = 0;
	};
}
