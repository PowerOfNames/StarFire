#include "sfpch.h"
#include "StarFire/Core/Identifier.h"

#include <random>

namespace StarFire {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDist;

	UniversalID::UniversalID()
		:m_Id(s_UniformDist(s_Engine))
	{
	}

	UniversalID::UniversalID(uint64_t id)
		: m_Id(id)
	{
	}
}