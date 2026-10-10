#pragma once
#include "Aurora/Renderer/Types.h"

#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

#include <string>

namespace Aurora {

	struct GraphicsPipelineSpecification
	{
		std::string Name;

		WindingOrder WindingOrder = WindingOrder::COUNTER_CLOCKWISE;
		PrimitiveTopology PrimitiveTopology = PrimitiveTopology::TRIANGLE_LIST;
		PolygonMode PolygonMode = PolygonMode::FILL;
		VertexInputRate VertexInputRate = VertexInputRate::VERTEX;
		bool IsBindless = false;
	};

	class GraphicsPipeline : public Substrate::RefCounted, public Substrate::Base<GraphicsPipeline>
	{
		DECLARE_TYPE(GraphicsPipeline, nullptr);
	public:

		virtual ~GraphicsPipeline() = default;
		virtual bool Compile() = 0;
		virtual void Destroy() = 0;

		static Ref<GraphicsPipeline> Create(const GraphicsPipelineSpecification& specs);
		virtual const GraphicsPipelineSpecification& GetSpecification() const = 0;
	};
}
