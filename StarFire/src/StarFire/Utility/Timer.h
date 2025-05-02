#pragma once

#include <chrono>
namespace StarFire {
	namespace Utils {

		class Timer
		{
		public:
			Timer()
			{
				m_StartTime = std::chrono::high_resolution_clock::now();
				m_Timestamp = m_StartTime;
			}
			~Timer() = default;

			inline double ElapsedTime() const { return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_StartTime)).count(); }
			inline float ElapsedTimeMilli() const { return ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_StartTime)).count() * 1000.0f; }
			inline double ElapsedTimeMicro() const { return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_StartTime)).count() * 1000000.0; }
			inline double ElapsedTimeNano() const { return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_StartTime)).count() * 1000000000.0; }

			void ResetTimestamp() { m_Timestamp = std::chrono::high_resolution_clock::now(); }
			inline double Timestamp() const { return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count(); }
			inline float TimestampMilli() const { return ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count() * 1000.0f; }
			inline double TimestampMicro() const { return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count() * 1000000.0; }
			inline double TimestampNano() const { return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count() * 1000000000.0; }


		private:
			std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
			std::chrono::time_point<std::chrono::steady_clock> m_Timestamp;
		};

	}
}
