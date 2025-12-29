#pragma once
#include "Substrate/Defines.h"

#include <atomic>
#include <concepts>
#include <cstdint>

namespace Substrate {

	/// <summary>
	/// Must be derived from and then used with RefPtr to provide reference counting functionality.
	/// </summary>
	class RefCounted
	{
	public:
		RefCounted(const RefCounted&) = delete;
		RefCounted& operator=(const RefCounted&) = delete;
		RefCounted(RefCounted&&) = delete;
		RefCounted& operator=(RefCounted&&) = delete;

		void AddRef()
		{
			m_RefCount.fetch_add(1, std::memory_order_relaxed);
		}

		void DecRef()
		{
			if (m_RefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				delete this;
			}
		}

#if SUBSTRATE_DETAILS_ENABLED
		uint32_t GetRefCount() const
		{
			return m_RefCount.load(std::memory_order_relaxed);
		}
#endif
	protected:
		RefCounted() = default;
		virtual ~RefCounted() = default;

	private:
		std::atomic<uint32_t> m_RefCount{ 1 };
	};	
}
