#include "Aurora/Renderer/Vulkan/VulkanGraphicsPipeline.h"
#include "AuroraInternal.h"

#include "Aurora/Renderer/Vertex.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"

#include "Shaders/ShaderByteCodes.h"

namespace Aurora::VK {
	
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specs)
		: m_Specification(specs)
	{
	}

	bool VulkanGraphicsPipeline::Compile()
	{
		PROFILE_FUNCTION;

		if (AURORA_REQUIRE_FAILS(CreateDescriptorSets(), "Failed to create bindless descriptor set."))
			return false;

		if (AURORA_REQUIRE_FAILS(CreateVertexLayout(), "Failed to create uber vertex layout."))
			return false;

		if (AURORA_REQUIRE_FAILS(CreateGraphicsPipeline(), "Failed to create bindless graphics pipeline."))
			return false;

		return true;
	}

	void VulkanGraphicsPipeline::Destroy()
	{
		PROFILE_FUNCTION;


		Ref<VulkanContext> context = GetRenderContext();

		context->SubmitToMainDeletionQueue(
			[
				descriptorPool = m_DescriptorPool,
				descriptorSetLayout = m_DescriptorSetLayout,
				pipelineLayout = m_PipelineLayout,
				pipeline = m_Pipeline
			] (VkDevice device, VmaAllocator, const VkAllocationCallbacks* allocCbs)
			{
				vkDestroyDescriptorPool(device, descriptorPool, allocCbs);
				vkDestroyDescriptorSetLayout(device, descriptorSetLayout, allocCbs);
				vkDestroyPipeline(device, pipeline, allocCbs);
				vkDestroyPipelineLayout(device, pipelineLayout, allocCbs);
			});
		m_DescriptorPool = VK_NULL_HANDLE;
		m_DescriptorSetLayout = VK_NULL_HANDLE;
		m_Pipeline = VK_NULL_HANDLE;
		m_PipelineLayout = VK_NULL_HANDLE;
	}

	bool VulkanGraphicsPipeline::CreateVertexLayout()
	{
		PROFILE_FUNCTION;

		//TODO: automate this, later auto generate from shader reflection
		m_VertexInputAttributeDescriptions.resize(3);
		m_VertexInputAttributeDescriptions[0].location = 0;
		m_VertexInputAttributeDescriptions[0].binding = 0;
		m_VertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		m_VertexInputAttributeDescriptions[0].offset = offsetof(UberVertex, Position);

		m_VertexInputAttributeDescriptions[1].location = 1;
		m_VertexInputAttributeDescriptions[1].binding = 0;
		m_VertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		m_VertexInputAttributeDescriptions[1].offset = offsetof(UberVertex, Color);

		m_VertexInputAttributeDescriptions[2].location = 2;
		m_VertexInputAttributeDescriptions[2].binding = 0;
		m_VertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		m_VertexInputAttributeDescriptions[2].offset = offsetof(UberVertex, TexCoord);

		m_VertexInputBindingDescription.binding = 0;
		m_VertexInputBindingDescription.stride = sizeof(UberVertex);
		m_VertexInputBindingDescription.inputRate = Convert::ToVkVertexInputRate(m_Specification.VertexInputRate);

		return true;
	}


	bool VulkanGraphicsPipeline::CreateDescriptorSets()
	{
		PROFILE_FUNCTION;

		//TODO: refactor IsBindless || DescriptorsFromShader.
		// We only support bindless atm

		constexpr int STORAGE_BINDING = 0;
		constexpr int SAMPLER_BINDIG = 1;
		constexpr int IMAGE_BINDING = 2;

		Ref<VulkanContext> context = GetRenderContext();

		VkDevice device = context->GetLogicalDevice();
		const PhysicalDeviceLimits& limits = context->GetPhysicalDeviceLimits();
		const VkAllocationCallbacks* allocCallbacks = context->GetAllocationCallbacks();


		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, limits.DescriptorLimits.MaxPerStageDescriptorStorageBuffers },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, limits.DescriptorLimits.MaxPerStageDescriptorSampledImages },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, limits.DescriptorLimits.MaxPerStageDescriptorSampledImages }
		};

		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.pNext = nullptr;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		AURORA_VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, allocCallbacks, &m_DescriptorPool), VK_SUCCESS, "Failed to create bindless descriptor pool.");
		if (m_DescriptorPool == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, (uint64_t)m_DescriptorPool, "BindlessDescriptorPool");


		VkDescriptorSetLayoutBinding storageBinding{};
		storageBinding.binding = STORAGE_BINDING;
		storageBinding.descriptorCount = limits.DescriptorLimits.MaxPerStageDescriptorStorageBuffers;
		storageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storageBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding samplerBinding{};
		samplerBinding.binding = SAMPLER_BINDIG;
		samplerBinding.descriptorCount = limits.DescriptorLimits.MaxPerStageSamplers;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding imageBinding{};
		imageBinding.binding = IMAGE_BINDING;
		imageBinding.descriptorCount = limits.DescriptorLimits.MaxPerStageDescriptorSampledImages;
		imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		imageBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { storageBinding, samplerBinding, imageBinding };

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		bindingFlagsInfo.pNext = nullptr;
		VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
		std::vector<VkDescriptorBindingFlags> bindingFlags = { flags, flags, flags };
		bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = &bindingFlagsInfo;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		AURORA_VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, allocCallbacks, &m_DescriptorSetLayout), VK_SUCCESS, "Failed to create descriptor set layout.");
		if (m_DescriptorSetLayout == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t)m_DescriptorSetLayout, "DescriptorSetLayout");


		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;
		AURORA_VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet), VK_SUCCESS, "Failed to allocate bindless descriptor set.");
		if (m_DescriptorSet == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t)m_DescriptorSet, "DescriptorSet");

		return true;
	}	

	bool VulkanGraphicsPipeline::CreateGraphicsPipeline()
	{
		PROFILE_FUNCTION;


		// == Pipeline Layout ==
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.size = 128;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;


		Ref<VulkanContext> context = GetRenderContext();
		VkDevice device = context->GetLogicalDevice();
		const PhysicalDeviceLimits& limits = context->GetPhysicalDeviceLimits();
		const VkAllocationCallbacks* allocCallbacks = context->GetAllocationCallbacks();

		AURORA_VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, allocCallbacks, &m_PipelineLayout), VK_SUCCESS, "Failed to create pipeline layout.");
		if (m_PipelineLayout == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)m_PipelineLayout, "GraphicsPipelineLayout");


		//TODO: bindless uber-shader Shader Modules					
		VkShaderModuleCreateInfo vertInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		vertInfo.pNext = nullptr;
		vertInfo.flags = 0;
		vertInfo.codeSize = Shaders::BindlessTest_vert_size;
		vertInfo.pCode = reinterpret_cast<const uint32_t*>(&Shaders::BindlessTest_vert);

		VkShaderModule vertModule = VK_NULL_HANDLE;
		AURORA_VK_CHECK(vkCreateShaderModule(device, &vertInfo, nullptr, &vertModule), VK_SUCCESS, "Failed to create bindlessTest vertex shader module.");
		if (vertModule == VK_NULL_HANDLE)
			return false;

		VkShaderModuleCreateInfo fragInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		fragInfo.pNext = nullptr;
		fragInfo.flags = 0;
		fragInfo.codeSize = Shaders::BindlessTest_frag_size;
		fragInfo.pCode = reinterpret_cast<const uint32_t*>(&Shaders::BindlessTest_frag);

		VkShaderModule fragModule = VK_NULL_HANDLE;
		AURORA_VK_CHECK(vkCreateShaderModule(device, &fragInfo, nullptr, &fragModule), VK_SUCCESS, "Failed to create bindlessTest fragment shader module.");
		if (vertModule == VK_NULL_HANDLE)
			return false;

		//Shader stages
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		vertShaderStageInfo.pNext = nullptr;
		vertShaderStageInfo.flags = 0;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		fragShaderStageInfo.pNext = nullptr;
		fragShaderStageInfo.flags = 0;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


		//Vertex input state
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.pNext = nullptr;
		vertexInputInfo.flags = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexInputAttributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttributeDescriptions.data();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &m_VertexInputBindingDescription;

		//Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyInfo.pNext = nullptr;
		inputAssemblyInfo.flags = 0;
		inputAssemblyInfo.topology = Convert::ToVkPrimitiveTopology(m_Specification.PrimitiveTopology);
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		//Viewport
		// Not used (dynamic state)
		//VkViewport viewport{};
		//viewport.x = 0.0f;
		//viewport.y = 0.0f;
		//viewport.width = (float)m_Extent.width;
		//viewport.height = (float)m_Extent.height;
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;

		////Scissors
		//VkRect2D scissor{};
		//scissor.offset = { 0, 0 };
		//scissor.extent = m_Extent;

		//Danymic state
		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicStateInfo.pNext = nullptr;
		dynamicStateInfo.flags = 0;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportInfo.pNext = nullptr;
		viewportInfo.flags = 0;
		viewportInfo.scissorCount = 1;
		viewportInfo.viewportCount = 1;

		//Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationInfo.pNext = nullptr;
		rasterizationInfo.flags = 0;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = Convert::ToVkPolygonMode(m_Specification.PolygonMode);
		rasterizationInfo.lineWidth = 1.0f;
		rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInfo.frontFace = Convert::ToVkWindingOrder(m_Specification.WindingOrder);
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.depthBiasConstantFactor = 0.0f;
		rasterizationInfo.depthBiasClamp = 0.0f;
		rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		//Multisampling state
		VkPipelineMultisampleStateCreateInfo multiSampInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multiSampInfo.pNext = nullptr;
		multiSampInfo.flags = 0;
		multiSampInfo.sampleShadingEnable = VK_FALSE;
		multiSampInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multiSampInfo.minSampleShading = 1.0f;
		multiSampInfo.pSampleMask = nullptr;
		multiSampInfo.alphaToCoverageEnable = VK_FALSE;
		multiSampInfo.alphaToOneEnable = VK_FALSE;

		//DepthTesting (not used)
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthStencilInfo.pNext = nullptr;
		depthStencilInfo.flags = 0;
		//Depth
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //TODO GREATER_OR_EQUAL for infinite depth
		depthStencilInfo.depthWriteEnable = VK_TRUE;
		//Stencil
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		//TODO: bounds
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.maxDepthBounds = 0;
		depthStencilInfo.minDepthBounds = 0;

		//Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlendStateInfo.pNext = nullptr;
		colorBlendStateInfo.flags = 0;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachment;
		colorBlendStateInfo.blendConstants[0] = 0.0f;
		colorBlendStateInfo.blendConstants[1] = 0.0f;
		colorBlendStateInfo.blendConstants[2] = 0.0f;
		colorBlendStateInfo.blendConstants[3] = 0.0f;

		// == Dynamic Rendering ==
		//Because we need to know the color and depth/stencil attachment formats, we basically need a way to hash at least this
		// better the whole pipeline config, and upon RenderGraph->Compile(), check the required attachments/formats
		// to figure out if we need another pipeline or can reuse one. We basically also need to attach a Ref<GraphicsPipeline> 
		// to the RenderGraph before or during creation (probably per RenderPass, as these define the needed attachments)
		// and during compile of the RenderGraph check if RenderPAsses are compatible with the attached pipeline.
		// We can prebake a Bindless pipeline and have a Aurora::GetBindlessPipeline hook, which returns Ref<GraphicsPipeline>.

		std::vector<VkFormat> colorFormats = { VK_FORMAT_R8G8B8A8_UNORM };
		VkPipelineRenderingCreateInfo dynaRenderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		dynaRenderingInfo.pNext = nullptr;
		dynaRenderingInfo.pColorAttachmentFormats = colorFormats.data();
		dynaRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
		dynaRenderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT; //TODO: ?
		dynaRenderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED; //TODO: ?
		dynaRenderingInfo.viewMask = 0; //TODO;

		// == Pipeline ==
		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.pNext = &dynaRenderingInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = VK_NULL_HANDLE; //because of dynamic rendering

		pipelineInfo.flags = 0;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		//We hardcode vertices into the shader for proof of concept of the pipeline for now
		//TODO
		pipelineInfo.pVertexInputState = nullptr;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multiSampInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		AURORA_VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, allocCallbacks, &m_Pipeline), VK_SUCCESS, "Failed to create GraphicsPipeline.");
		if (m_Pipeline == VK_NULL_HANDLE)
			return false;
		AURORA_VK_ATTACH_DEBUG_NAME(device, VK_OBJECT_TYPE_PIPELINE, (uint64_t)m_Pipeline, "GraphicsPipeline");

		vkDestroyShaderModule(device, vertModule, allocCallbacks);
		vkDestroyShaderModule(device, fragModule, allocCallbacks);

		return true;
	}
}