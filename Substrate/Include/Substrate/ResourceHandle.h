#pragma once

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
	template<typename TType, typename Tag>
	struct ResourceHandle
	{
		static constexpr TType INVALID_HANDLE = static_cast<TType>(~0);

		constexpr ResourceHandle() noexcept : m_Handle(INVALID_HANDLE) {}
		constexpr ResourceHandle(TType handle) noexcept : m_Handle(handle) {}
		constexpr ResourceHandle(const ResourceHandle&) noexcept = default;
		constexpr ResourceHandle(ResourceHandle&&) noexcept = default;

		constexpr bool IsValid() const noexcept { return m_Handle != INVALID_HANDLE; }

		constexpr operator TType() const noexcept { return m_Handle; }
		constexpr bool operator==(const ResourceHandle& other) const noexcept { return m_Handle == other.m_Handle; }
		constexpr bool operator!=(const ResourceHandle& other) const noexcept { return m_Handle != other.m_Handle; }
		constexpr ResourceHandle& operator=(const ResourceHandle&) noexcept = default;

	private:
		TType m_Handle;
	};
}

namespace std {

	template<typename TType, typename Tag>
	struct hash<Substrate::ResourceHandle<TType, Tag>>
	{
		std::size_t operator()(const Substrate::ResourceHandle<TType, Tag>& handle) const
		{
			return hash<uint64_t>{}(static_cast<uint64_t>(handle));
		}
	};
}
