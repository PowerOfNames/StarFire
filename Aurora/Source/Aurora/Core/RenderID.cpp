#include "Aurora/RenderID.h"

#include <random>

namespace Aurora {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDist;

	RenderID::RenderID()
		:m_Id(s_UniformDist(s_Engine))
	{
	}

	RenderID::RenderID(uint64_t id)
		: m_Id(id)
	{
	}

}