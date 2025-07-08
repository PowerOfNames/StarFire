#pragma once
#include <cstdint>
#include <xhash>

namespace Aurora {

	class RenderID
	{
	public:
		RenderID();
		RenderID(uint64_t id);
		RenderID(const RenderID& rhs) = default;
		~RenderID() = default;

		constexpr operator uint64_t() const { return m_Id; }
		constexpr const bool operator==(const RenderID& other)
		{
			return other.m_Id == m_Id;
		}

	private:
		uint64_t m_Id = 0;
	};
}

namespace std {

	template<>
	struct hash<Aurora::RenderID>
	{
		std::size_t operator()(const Aurora::RenderID& id) const
		{
			return hash<uint64_t>()((uint64_t)id);
		}
	};
}