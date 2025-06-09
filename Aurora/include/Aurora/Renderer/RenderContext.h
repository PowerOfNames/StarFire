#pragma once
#include "Aurora/Renderer/APIType.h"
#include "Aurora/Renderer/WSIPlatform.h"

#include <memory>
#include <string>

namespace Aurora {

	struct RenderContextSpecification
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
		
		APIType API = APIType::API_TYPE_NONE;

		struct SurfaceSpecification
		{
			WSIPlatformType WSI = WSIPlatformType::SURFACE_PLAFORM_NONE;
			void* WindowHandle = nullptr;
			uint8_t FramesPerFlight = 1;
		} SurfaceSpecs;

		struct InstanceSpecification
		{
			bool EnableDebugUtils = false;
			
		} InstanceSpecs;
	};


	class RenderContext 
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual const RenderContextSpecification& GetSpecification() const = 0;

		static std::shared_ptr<RenderContext> Create(const RenderContextSpecification& contextSpecs);
	};

}
