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


	void Renderer::Init()
	{



		AURORA_INFO("Renderer initialized");
	}


	void Renderer::Shutdown()
	{


		AURORA_INFO("Renderer shutdown");
	}

}
