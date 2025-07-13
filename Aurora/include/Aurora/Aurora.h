#pragma once
#include "Aurora/Logging/LogLevel.h"

#include "Aurora/Renderer/AssetHandles.h"

#include "Aurora/Renderer/RenderContextSpecification.h"

#include "Aurora/AppSettings.h"

#include <atomic>
#include <string>
#include <string_view>

#define ENABLE_TRACE 0

namespace Aurora {

	// ========== Logging ==========
	using LogCallback = void(*)(LogLevel, const std::string&, const char* file, const char* func, int line);
	using RegistryRegisterCallback = void(*)(const std::string& typeName, std::atomic<uint64_t>*);
	using RegistryUnregisterCallback = void(*)(const std::string& typeName);

	void SetLoggingCallback(LogCallback callback);
	void SetRefRegistryRegisterCallback(RegistryRegisterCallback callback);
	void SetRefRegistryUnregisterCallback(RegistryUnregisterCallback callback);

	// ========== Render Context ==========
	bool InitializeRenderContext(const RenderContextSpecification& contextSpecs);

	bool BeginFrame();
	void EndFrame();
	void SwapFrame();
	void Resize(uint32_t width, uint32_t height);

	bool Shutdown();
	
	// ========== Settings ==========
	inline AppSettings& ChangeAppSettings() { return AppSettings::Instance(); }
}
