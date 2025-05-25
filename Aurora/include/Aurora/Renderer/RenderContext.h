#pragma once
#include "Aurora/Renderer/APIType.h"

#include <memory>

namespace Aurora {

	struct RenderContextSpecification
	{
		APIType API = APIType::API_TYPE_NONE;

	};


	class RenderContext {
	public:
		virtual ~RenderContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual const RenderContextSpecification& GetSpecification() const = 0;

		static std::shared_ptr<RenderContext> Create(const RenderContextSpecification& contextSpecs);
	};

}
