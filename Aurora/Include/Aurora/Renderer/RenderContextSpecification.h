#pragma once
#include "Aurora/Renderer/WSIPlatform.h"

#include <string>

namespace Aurora
{
	struct InitializationSpecification
	{
		std::string AppName = "Sandbox";

		//TODO: determine metric for version numbering and usage
		struct ApplicationVersionNumber
		{
			uint32_t Major = 1;
			uint32_t Minor = 0;
			uint32_t Patch = 0;
		} AppVersion;

		//TODO: determine metric for version numbering and usage		
		struct AuroraVersionNumber
		{
			uint32_t Major = 1;
			uint32_t Minor = 0;
			uint32_t Patch = 0;
		} AuroraVersion;

		struct SurfaceSpecification
		{
			WSIPlatformType WSI = WSIPlatformType::SURFACE_PLAFORM_NONE;
			void* WindowHandle = nullptr;
			uint8_t FramesPerFlight = 1;
			bool VSync = false;
			uint32_t Width = 0;
			uint32_t Height = 0;
			uint32_t FramebufferWidth = 0;
			uint32_t FramebufferHeight = 0;

			struct ClearColor
			{
				float R = 0.0f;
				float G = 0.0f;
				float B = 0.0f;
				float A = 1.0f;
			} ClearColor;

		} SurfaceSpecs;

		struct InstanceSpecification
		{
			bool EnableDebugUtils = false;
			bool EnableInfoDebugLevel = false;

		} InstanceSpecs;
	};
}
