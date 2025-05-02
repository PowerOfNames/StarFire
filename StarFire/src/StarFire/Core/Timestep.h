#pragma once

namespace StarFire {

	//This class is a thin wrapper around a double time duration to provide simple functionality
	//for conversion from seconds (inuput) to milli, micro or nano seconds
	class Timestep
	{
	public:
		Timestep(double durationInSeconds = 0.0)
			: m_DurationInS(durationInSeconds)
		{
		}
		~Timestep() = default;

		operator double() const { return m_DurationInS; }
		operator float() const { return static_cast<float>(m_DurationInS); }
		
		inline double InSec() { return m_DurationInS; }
		inline float InMilli() { return static_cast<float>(m_DurationInS) * 1000.0f; }
		inline float InMicro() { return static_cast<float>(m_DurationInS) * 1000000.0f; }

	private:
		double m_DurationInS;
	};
}
