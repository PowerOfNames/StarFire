#pragma once
#include "Aurora/Renderer/GraphicsPipeline.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"


#include <vector>

namespace Aurora::VK {

	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
		DECLARE_TYPE(VulkanGraphicsPipeline, GraphicsPipeline::GetStaticTypeInfo());

	public:
		VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specs);
		virtual ~VulkanGraphicsPipeline() = default;
		
		virtual bool Compile() override;
		virtual void Destroy() override;
		
		inline VkPipeline GetPipelineHandle() const { return m_Pipeline; }

		const GraphicsPipelineSpecification& GetSpecification() const override { return m_Specification; }
	
	private:
		bool CreateGraphicsPipeline();

	private:
		GraphicsPipelineSpecification m_Specification;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
	};
 }
