#pragma once
#include "Aurora/Renderer/Handles.h"
#include "Aurora/Renderer/Image.h"

#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

#include <string>
#include <vector>

namespace Aurora {


	struct RenderPassSpecification
	{
		std::string Name = "Render pass name";

		std::vector<ImageSpecification> ColorAttachments;
		ImageSpecification DepthAttachment{};
	};
	
	class RenderPass : public Substrate::RefCounted
	{
	public:
		virtual ~RenderPass() = default;
		virtual void Destroy() = 0;

		virtual const ImageHandle GetColorAttachmentHandle(uint32_t index = 0) const = 0;
		virtual const ImageHandle GetDepthAttachmentHandle() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specs);
		virtual const RenderPassSpecification& GetSpecification() const = 0;
	};
}
