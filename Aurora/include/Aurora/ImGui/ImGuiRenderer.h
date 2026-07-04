#pragma once
#include "Aurora/Renderer/Handles.h"

#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"


#include <cstdint>

namespace Aurora {

	class ImGuiRenderer : public Substrate::RefCounted
	{
	public:
		~ImGuiRenderer() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void BeginFrame() = 0;
		virtual void OnWindowResize(uint32_t width, uint32_t height) = 0;
		virtual void EndFrame() = 0;
		virtual uint64_t GetTextureIDFromHandle(ImageHandle image) = 0;
		virtual void ReturnTextureIDFromHandle(ImageHandle image) = 0;


		static Ref<ImGuiRenderer> Create();
	};
}
