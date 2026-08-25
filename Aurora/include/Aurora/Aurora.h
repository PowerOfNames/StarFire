#pragma once
#include "Aurora/Logging/LogLevel.h"
#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/Image.h"
#include "Aurora/Renderer/Buffer.h"

#include "Aurora/Renderer/RenderContextSpecification.h"

#include "Aurora/AppSettings.h"

#include <atomic>
#include <string>

namespace Aurora {

	// ========== Settings ==========
	inline AppSettings& ChangeAppSettings() { return AppSettings::Instance(); }

	// ========== Logging ==========
	using LogCallback = void(*)(LogLevel, const std::string&, const char* file, const char* func, int line);
	using RegistryRegisterCallback = void(*)(const std::string& typeName, std::atomic<uint64_t>*);
	using RegistryUnregisterCallback = void(*)(const std::string& typeName);

	void SetLoggingCallback(LogCallback callback);
	void SetRefRegistryRegisterCallback(RegistryRegisterCallback callback);
	void SetRefRegistryUnregisterCallback(RegistryUnregisterCallback callback);

	// ========== Render Context ==========
	bool Initialize(const InitializationSpecification& contextSpecs);
	bool Shutdown();

	bool BeginFrame();
	void EndFrame();
	void SwapFrame();
	void Resize(uint32_t width, uint32_t height);
	
	// ========== Resources ==========
	ImageHandle CreateImage(const ImageSpecification& imageSpecs);
	bool IsHandleValid(ImageHandle handle);
	void DestroyImage(ImageHandle handle);
	
	BufferHandle CreateBuffer(const BufferSpecification& bufferSpecs);
	void DestroyBuffer(BufferHandle handle);

	VertexBufferHandle CreateVertexBuffer(const VertexBufferSpecification& bufferSpecs);
	void DestroyVertexBuffer(VertexBufferHandle handle);

	IndexBufferHandle CreateIndexBuffer(const IndexBufferSpecification& bufferSpecs);
	void DestroyIndexBuffer(IndexBufferHandle handle);
}
