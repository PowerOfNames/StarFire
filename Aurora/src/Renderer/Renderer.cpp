#include "Aurora/Renderer.h"
#include "Aurora/Logging.h"

#include <assert.h>

namespace Aurora {


	Renderer::Renderer()
	{
		//Check if Log-Callback exists
		assert(Log::HasCallbackSet(), "Aurora::Log::SetCallback must be called before Renderer creation!");
	}


	Renderer::~Renderer()
	{
	}


	void Renderer::Init(const char* message)
	{
		m_Message = message;
		AURORA_INFO("Hi from Aurora {}", message);
	}


	void Renderer::Shutdown()
	{
		AURORA_INFO("Hi from Aurora {}{}", m_Message, "!");
	}

}
