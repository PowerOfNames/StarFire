#include "Core/Logging.h"

namespace Aurora {

	void Log::SetCallback(LogCallbackFn callback)
	{
		s_Callback = callback;
	}

	void Log::Message(LogLevel level, const std::string& message)
	{
		if (s_Callback)
			s_Callback(level, message);
	}

	void Log::Shutdown()
	{
		s_Callback = nullptr;
	}
}

