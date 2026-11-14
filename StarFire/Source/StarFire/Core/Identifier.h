#pragma once

#include <cstdint>
#include <functional>

namespace StarFire {

	class UniversalID
	{
	public:
		UniversalID();
		explicit UniversalID(uint64_t id);
		UniversalID(const UniversalID& rhs) = default;
		~UniversalID() = default;

		constexpr operator uint64_t() const {
			return m_Id;
		}
		constexpr bool operator==(const UniversalID& other) const
		{
			return other.m_Id == m_Id;
		}
		constexpr bool operator!=(const UniversalID& other) const
		{
			return !(*this == other);
		}

	private:
		uint64_t m_Id = 0;
	};
}

namespace std {

	template<>
	struct hash<StarFire::UniversalID>
	{
		std::size_t operator()(const StarFire::UniversalID& id) const
		{
			return hash<uint64_t>()((uint64_t)id);
		}
	};
}
