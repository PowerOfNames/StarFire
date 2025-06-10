#include "Core/Logging.h"

namespace Aurora {

	void Log::SetCallback(LogCallbackFn callback)
	{
		s_Callback = callback;
	}

	void Log::Message(LogLevel level, const std::string& message, const char* file, const char* func, int line)
	{
		if (s_Callback)
			s_Callback(level, message, file, func, line);
	}

	void Log::Shutdown()
	{
		s_Callback = nullptr;
	}
}

