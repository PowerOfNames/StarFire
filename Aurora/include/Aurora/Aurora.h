#pragma once
#include "Aurora/Logging/LogLevel.h"

#include "Renderer/Handles.h"

#include "Aurora/ApplicationSettings.h"
#include "Aurora/Renderer/RenderContextSpecification.h"

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
	ApplicationSettings& ChangeSettings();
	bool InitializeRenderContext(const RenderContextSpecification& contextSpecs);

	bool BeginFrame();
	void EndFrame();
	void SwapFrame();
	void Resize(uint32_t width, uint32_t height);

	bool Shutdown();

	// ========== Resources ==========
	ShaderAssetHandle LoadShader(std::string_view name);
	
}
