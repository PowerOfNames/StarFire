#pragma once
#include "Aurora/Logging/LogLevel.h"

#include "Renderer/RenderContext.h"

#include <atomic>
#include <string>

#define ENABLE_TRACE 0

namespace Aurora {

	static std::unique_ptr<RenderContext> s_RenderContext = nullptr;
	
	using LogCallback = void(*)(LogLevel, const std::string&, const char* file, const char* func, int line);
	using RegistryRegisterCallback = void(*)(const std::string& typeName, std::atomic<uint64_t>*);
	using RegistryUnregisterCallback = void(*)(const std::string& typeName);

	void SetLoggingCallback(LogCallback callback);
	void SetRefRegistryRegisterCallback(RegistryRegisterCallback callback);
	void SetRefRegistryUnregisterCallback(RegistryUnregisterCallback callback);

	bool InitializeRenderContext(const RenderContextSpecification& contextSpecs);

	bool BeginFrame();
	void EndFrame();
	void SwapFrame();
	void Resize(uint32_t width, uint32_t height);

	bool Shutdown();
}
