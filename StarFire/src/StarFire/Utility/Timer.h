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
			inline float ElapsedTimeMicro() const { return ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_StartTime)).count() * 1000000.0f; }
			inline float ElapsedTimeNano() const { return ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_StartTime)).count() * 1000000000.0f; }

			// Resets Timestamp, not Start Time
			void ResetTimestamp() { m_Timestamp = std::chrono::high_resolution_clock::now(); }

			//Retrieves time delta since last Timestamp call in seconds, then resets timestamp
			inline double Timestamp() 
			{
				double delta = ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count(); 
				ResetTimestamp();
				return delta; 
			}
			//Retrieves time delta since last Timestamp call in milliseconds, then resets timestamp
			inline float TimestampMilli()
			{
				float delta = ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count() * 1000.0f;
				ResetTimestamp(); 
				return delta;
			}
			//Retrieves time delta since last Timestamp call in microseconds, then resets timestamp
			inline float TimestampMicro() 
			{
				float delta = ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count() * 1000000.0f; 
				ResetTimestamp(); 
				return delta;
			}
			//Retrieves time delta since last Timestamp call in nanoseconds, then resets timestamp
			inline float TimestampNano()
			{
				float delta = ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - m_Timestamp)).count() * 1000000000.0f; 
				ResetTimestamp(); 
				return delta;
			}

		private:
			std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
			std::chrono::time_point<std::chrono::steady_clock> m_Timestamp;
		};

	}
}
