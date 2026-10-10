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
		inline VkPipelineLayout GetPipelineLayoutHandle() const { return m_PipelineLayout; }
		inline const VkDescriptorSet GetDescriptorSet() const {return m_DescriptorSet; }

		const GraphicsPipelineSpecification& GetSpecification() const override { return m_Specification; }
	
	private:
		bool CreateDescriptorSets();
		bool CreateGraphicsPipeline();
		bool CreateVertexLayout();

	private:
		GraphicsPipelineSpecification m_Specification;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;


		std::vector<VkVertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
		VkVertexInputBindingDescription m_VertexInputBindingDescription{};
	};
 }
