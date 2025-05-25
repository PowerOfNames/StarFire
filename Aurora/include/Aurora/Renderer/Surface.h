#pragma once

#include "Aurora/Renderer/APIType.h"
#include "Aurora/Renderer/WSIPlatform.h"

#include <memory>

namespace Aurora {	

	struct SurfaceSpecification
	{
		WSIPlatformType TargetPlatform = WSIPlatformType::SURFACE_PLAFORM_NONE;
		APIType API = APIType::API_TYPE_NONE;
	};

	class Surface
	{
	public:
		virtual ~Surface() = default;

		virtual void Init() = 0;

		virtual const SurfaceSpecification& GetSpecifications() const = 0;

		static std::shared_ptr<Surface> Create(const SurfaceSpecification& specs);
	};
}
