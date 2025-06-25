#pragma once

#include "Core/RefRegistry.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>

namespace Aurora {

	template<typename T>
	class RefCounted : public std::enable_shared_from_this<T>
	{
	public:
		~RefCounted()
		{
			s_RefCount.fetch_add(-1);
			std::lock_guard<std::mutex> lock(m_Mutex);
			if (static_cast<uint64_t>(s_RefCount.load()) == 0)
				RefRegistry::Unregister(T.GetTypeName());
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

		virtual const std::string GetTypeName() const = 0;

	protected:
		RefCounted()
		{
			s_RefCount.fetch_add(1);
			std::lock_guard<std::mutex> lock(m_Mutex);
			if (static_cast<uint64_t>(s_RefCount.load() == 1))
				RefRegistry::Register(T.GetTypeName(), &s_RefCount);
		}
		RefCounted<T>(const RefCounted<T>&) = default;
		RefCounted<T>& operator=(const RefCounted<T>&) = default;

	private:
		inline static std::atomic<uint64_t> s_RefCount = 0;

		std::mutex m_Mutex;
	};
}
