#pragma once

#include <type_traits>

namespace Substrate {

	/// <summary>
	/// Use like this:
	/// Define tag: 
	/// struct MyResourceTag {};
	/// Specify handle:
	/// using MyHandle = ResourceHandle<uint32_t, MyResourceTag>;
	/// This way, MyHandle is distinct from other ResourceHandle types even if they use the same underlying type (uint32_t in this case).
	/// </summary>
	/// <typeparam name="TType"></typeparam>
	/// <typeparam name="Tag"></typeparam>
	template<typename TType, typename TTag>
	struct ResourceHandle
	{
		using Type = TType;
		using Tag = TTag;

		static constexpr TType INVALID_HANDLE = static_cast<TType>(~0);

		constexpr ResourceHandle() noexcept : m_Handle(INVALID_HANDLE) {}
		constexpr ResourceHandle(TType handle) noexcept : m_Handle(handle) {}
		constexpr ResourceHandle(const ResourceHandle&) noexcept = default;
		constexpr ResourceHandle(ResourceHandle&&) noexcept = default;

		constexpr bool IsValid() const noexcept { return m_Handle != INVALID_HANDLE; }

		constexpr operator TType() const noexcept { return m_Handle; }
		//constexpr bool operator==(const ResourceHandle& other) const noexcept { return m_Handle == other.m_Handle; }
		//constexpr bool operator!=(const ResourceHandle& other) const noexcept { return m_Handle != other.m_Handle; }
		constexpr ResourceHandle& operator=(const ResourceHandle&) noexcept = default;

		template<typename TOtherResourceHandle>
		constexpr TOtherResourceHandle As() const noexcept
		{
			static_assert(std::is_same_v< typename TOtherResourceHandle::Tag, Tag > || 
						  std::is_base_of_v<typename TOtherResourceHandle::Tag, Tag> || 
						  std::is_base_of_v<Tag, typename TOtherResourceHandle::Tag>, 
						  "Can only assign from another ResourceHandle with the same underlying tag.");
			static_assert(std::is_convertible_v<typename TOtherResourceHandle::Type, Type>, 
						  "Can only assign from another ResourceHandle with the same underlying type.");

			return TOtherResourceHandle(m_Handle);
		}

	private:
		TType m_Handle;
	};
}

#include <functional>

namespace std {

	template<typename TType, typename TTag>
	struct hash<Substrate::ResourceHandle<TType, TTag>>
	{
		std::size_t operator()(const Substrate::ResourceHandle<TType, TTag>& handle) const
		{
			return hash<uint64_t>{}(static_cast<uint64_t>(handle));
		}
	};
}
