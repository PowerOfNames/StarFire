#pragma once
#include "Aurora/Renderer/RenderGraph.h"
#include "Aurora/Renderer/GraphicsPipeline.h"
#include "Aurora/Renderer/Handles.h"

#include "Aurora/ShaderDataTypes.h"
#include "Aurora/Renderer/Vulkan/VulkanCore.h"

#include "Substrate/RefPtr.h"
#include "Substrate/UtilityFunctions.h"

#include <vector>

namespace Aurora::VK {

	constexpr uint32_t MAX_DRAWS_PER_FRAME = 1024;

	struct CompiledAttachment
	{
		std::string Name;
		ImageHandle Handle;
		VkRenderingAttachmentInfo AttachmentInfo;
		bool CopyRequested = false;
	};

	struct CompiledPassSlot
	{
		std::vector<CompiledAttachment> ColorAttachments;
		CompiledAttachment DepthAttachment{};
	};

	struct CompiledPass
	{
		std::string Name;
		//one per frame in flight
		std::vector<CompiledPassSlot> FiFSlots;
		VkRect2D RenderArea{};
		bool HasDepthAttachment = false;
	};

	struct CompiledAttachmentCopy
	{
		std::string PassName;
		std::string AttachmentName;
		std::vector<ImageHandle> CopySourcesPerFif;
		std::vector<ImageHandle> CopyTargetsPerFif;
	};

	class VulkanRenderGraph : public RenderGraph
	{
		DECLARE_TYPE(VulkanRenderGraph, RenderGraph::GetStaticTypeInfo());

	public:
		VulkanRenderGraph(const RenderGraphSpecification& specs);
		virtual ~VulkanRenderGraph() = default;

		virtual void Destroy() override;
		virtual void OnResize(uint32_t width, uint32_t height) override;

		virtual void AddRenderPass(const Ref<RenderPass>& renderPass) override;
		virtual void AddAttachmentCopy(std::string_view copyRequestName, const AttachmentCopyRequest& copyInfo) override;
		virtual void RemoveAttachmentCopy(std::string_view copyRequestName) override;
		virtual ImageHandle GetCopyTarget(std::string_view copyRequestName) override;

		virtual inline void SetBindlessPipeline(const Ref<GraphicsPipeline>& pipeline) override { m_BindlessPipeline = pipeline; }


		virtual bool Compile() override;

		virtual void Execute(const VertexBufferHandle vbHandle, const IndexBufferHandle ibHandle, const ImageHandle texture, const void* globalData, size_t globalDataSize) override;

		virtual inline const RenderGraphSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void DrawObjects(VkCommandBuffer cmd, VertexBufferHandle vertices, IndexBufferHandle indices, uint8_t frameIdx, ImageHandle textureHandle, size_t globalsAddress);
		bool CreateDataBuffers();
		void ClearCompilations();
		void CompileCopyRequest(std::string_view request);
		bool CheckAndTransitImage(VulkanImageData* imageData, VkCommandBuffer cmd, VkImageLayout targetLayout);
		bool CheckObjectDataAvailability(VulkanBufferData* vertexData, VulkanBufferData* indexData);
		bool CheckBindlessAvailability(VulkanBufferData* drawData, VulkanBufferData* materials);
		const std::vector<CompiledAttachment*> FindAttachmentByNameInPass(std::string_view passName, std::string_view attachmentName);

#if defined(AURORA_DEBUG_MODE)
		bool ValidateCompiledGraph();
#endif

	private:
		RenderGraphSpecification m_Specification;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		bool m_NeedsResize = false;
		bool m_Compiled = false;

		std::vector<Ref<RenderPass>> m_RenderPasses;
		std::unordered_map<std::string, AttachmentCopyRequest> m_CopyRequests;

		//Matched passes with copy requests
		std::unordered_map<std::string, CompiledAttachmentCopy> m_CompiledCopies;
		std::vector<CompiledPass> m_CompiledPasses;

		Ref<GraphicsPipeline> m_BindlessPipeline;

		static constexpr size_t DRAW_DATA_BYTES = Substrate::Utility::AlignUpToMultipleOfMinAlignment(sizeof(GPU::BindlessDrawData), AURORA_GPU_REF_ALIGN) * MAX_DRAWS_PER_FRAME;
		static constexpr size_t MATERIAL_BYTES = Substrate::Utility::AlignUpToMultipleOfMinAlignment(sizeof(GPU::BindlessMaterial) * MAX_DRAWS_PER_FRAME, AURORA_GPU_REF_ALIGN);
		BufferHandle m_BindlessGlobalsBuffer = BufferHandle::INVALID_HANDLE;
		BufferHandle m_BindlessDrawDataBuffer = BufferHandle::INVALID_HANDLE;
		BufferHandle m_BindlessMaterialBuffer = BufferHandle::INVALID_HANDLE;
	};
}