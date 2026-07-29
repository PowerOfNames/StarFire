#include <catch2/catch_test_macros.hpp>

#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"

// These cover Aurora::VK::Convert, the only bucket in the Vulkan Utility
// folder that is a pure function of its arguments: no device, no allocator,
// no command buffer. That is what makes it testable without a GPU.

using namespace Aurora;
namespace Cv = Aurora::VK::Convert;


TEST_CASE("Convert: Aurora enums alias their Vulkan counterparts", "[convert][types]")
{
	SECTION("Format")
	{
		CHECK(Cv::ToVkFormat(Format::UNKNOWN) == VK_FORMAT_UNDEFINED);
		CHECK(Cv::ToVkFormat(Format::RGBA8_UNORM) == VK_FORMAT_R8G8B8A8_UNORM);
		CHECK(Cv::ToVkFormat(Format::RGBA8_SRGB) == VK_FORMAT_R8G8B8A8_SRGB);
		CHECK(Cv::ToVkFormat(Format::DEPTH32_SFLOAT) == VK_FORMAT_D32_SFLOAT);
		CHECK(Cv::ToVkFormat(Format::DEPTH24_STENCIL8) == VK_FORMAT_D24_UNORM_S8_UINT);
	}

	SECTION("ImageTiling")
	{
		CHECK(Cv::ToVkImageTiling(ImageTiling::OPTIMAL) == VK_IMAGE_TILING_OPTIMAL);
		CHECK(Cv::ToVkImageTiling(ImageTiling::LINEAR) == VK_IMAGE_TILING_LINEAR);
	}

	SECTION("MemoryUsage")
	{
		CHECK(Cv::ToVmaMemoryUsage(MemoryUsage::UNKNOWN) == VMA_MEMORY_USAGE_UNKNOWN);
		CHECK(Cv::ToVmaMemoryUsage(MemoryUsage::GPU_ONLY) == VMA_MEMORY_USAGE_GPU_ONLY);
		CHECK(Cv::ToVmaMemoryUsage(MemoryUsage::CPU_ONLY) == VMA_MEMORY_USAGE_CPU_ONLY);
		CHECK(Cv::ToVmaMemoryUsage(MemoryUsage::CPU_TO_GPU) == VMA_MEMORY_USAGE_CPU_TO_GPU);
		CHECK(Cv::ToVmaMemoryUsage(MemoryUsage::GPU_TO_CPU) == VMA_MEMORY_USAGE_GPU_TO_CPU);
	}

	SECTION("ImageUsageFlags, including combinations")
	{
		CHECK(Cv::ToVkImageUsageFlags(ImageUsageFlags::NONE) == 0u);
		CHECK(Cv::ToVkImageUsageFlags(ImageUsageFlags::TRANSFER_SRC) == VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		CHECK(Cv::ToVkImageUsageFlags(ImageUsageFlags::SAMPLED) == VK_IMAGE_USAGE_SAMPLED_BIT);
		CHECK(Cv::ToVkImageUsageFlags(ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

		const ImageUsageFlags combined = ImageUsageFlags::COLOR_ATTACHMENT | ImageUsageFlags::TRANSFER_SRC;
		CHECK(Cv::ToVkImageUsageFlags(combined) == (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
	}

	SECTION("BufferUsageFlags, including the high bit")
	{
		CHECK(Cv::ToVkBufferUsageFlags(BufferUsageFlags::NONE) == 0u);
		CHECK(Cv::ToVkBufferUsageFlags(BufferUsageFlags::VERTEX_BUFFER) == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		CHECK(Cv::ToVkBufferUsageFlags(BufferUsageFlags::INDIRECT_BUFFER) == VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
		CHECK(Cv::ToVkBufferUsageFlags(BufferUsageFlags::SHADER_DEVICE_ADDRESS) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

		const BufferUsageFlags combined = BufferUsageFlags::VERTEX_BUFFER | BufferUsageFlags::TRANSFER_DST;
		CHECK(Cv::ToVkBufferUsageFlags(combined) == (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT));
	}
}


TEST_CASE("Convert: buffer usage to pipeline stage", "[convert][stage]")
{
	SECTION("an explicit queue short-circuits the usage bits entirely")
	{
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::VERTEX_BUFFER, VK::QueueOwner::TRANSFER) == VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::VERTEX_BUFFER, VK::QueueOwner::COMPUTE) == VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
	}

	SECTION("per-usage stages")
	{
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::NONE) == 0u);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::VERTEX_BUFFER) == VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::INDEX_BUFFER) == VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::INDIRECT_BUFFER) == VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_SRC) == VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::TRANSFER_DST) == VK_PIPELINE_STAGE_2_TRANSFER_BIT);

		constexpr VkPipelineStageFlags2 allShaders =
			VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::UNIFORM_BUFFER) == allShaders);
		CHECK(Cv::GetStageFromBufferUsage(BufferUsageFlags::STORAGE_BUFFER) == allShaders);
	}

	SECTION("combined usages OR their stages together")
	{
		const BufferUsageFlags usage = BufferUsageFlags::VERTEX_BUFFER | BufferUsageFlags::INDIRECT_BUFFER;
		CHECK(Cv::GetStageFromBufferUsage(usage) == (VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT));
	}
}


TEST_CASE("Convert: buffer usage to access mask", "[convert][access]")
{
	CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::NONE) == 0u);
	CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::VERTEX_BUFFER) == VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT);
	CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::INDEX_BUFFER) == VK_ACCESS_2_INDEX_READ_BIT);
	CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::UNIFORM_BUFFER) == VK_ACCESS_2_UNIFORM_READ_BIT);
	CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::INDIRECT_BUFFER) == VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT);
	CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::STORAGE_BUFFER) == (VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT));

	SECTION("transfer direction is distinguished, unlike the image variant")
	{
		CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::TRANSFER_SRC) == VK_ACCESS_2_TRANSFER_READ_BIT);
		CHECK(Cv::GetAccessFromBufferUsage(BufferUsageFlags::TRANSFER_DST) == VK_ACCESS_2_TRANSFER_WRITE_BIT);
	}
}


TEST_CASE("Convert: image usage to pipeline stage", "[convert][stage]")
{
	SECTION("an explicit queue short-circuits the usage bits entirely")
	{
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::SAMPLED, VK::QueueOwner::TRANSFER) == VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::SAMPLED, VK::QueueOwner::COMPUTE) == VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
	}

	SECTION("per-usage stages")
	{
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::NONE) == 0u);
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::COLOR_ATTACHMENT) == VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::TRANSFER_SRC) == VK_PIPELINE_STAGE_2_TRANSFER_BIT);

		constexpr VkPipelineStageFlags2 allShaders =
			VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::SAMPLED) == allShaders);
		CHECK(Cv::GetStageFromImageUsage(ImageUsageFlags::STORAGE) == allShaders);
	}

	SECTION("a depth-only image gets fragment-test stages and no colour stage")
	{
		const VkPipelineStageFlags2 stages = Cv::GetStageFromImageUsage(ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT);
		CHECK(stages == (VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT));
		CHECK((stages & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) == 0u);
	}
}


TEST_CASE("Convert: image usage to access mask", "[convert][access]")
{
	CHECK(Cv::GetAccessFromImageUsage(ImageUsageFlags::NONE) == 0u);
	CHECK(Cv::GetAccessFromImageUsage(ImageUsageFlags::SAMPLED) == VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
	CHECK(Cv::GetAccessFromImageUsage(ImageUsageFlags::STORAGE) == (VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT));
	CHECK(Cv::GetAccessFromImageUsage(ImageUsageFlags::COLOR_ATTACHMENT) == (VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT));

	SECTION("a depth-only image gets depth access and no colour access")
	{
		const VkAccessFlags2 access = Cv::GetAccessFromImageUsage(ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT);
		CHECK(access == (VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
		CHECK((access & VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT) == 0u);
	}

	SECTION("transfer contributes access bits, not stage bits")
	{
		const VkAccessFlags2 access = Cv::GetAccessFromImageUsage(ImageUsageFlags::TRANSFER_SRC);
		CHECK(access == (VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT));
	}
}


TEST_CASE("Convert: format to aspect mask", "[convert][aspect]")
{
	SECTION("colour formats")
	{
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_R8G8B8A8_UNORM) == VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_R8G8B8A8_SRGB) == VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_B8G8R8A8_UNORM) == VK_IMAGE_ASPECT_COLOR_BIT);
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_B8G8R8A8_SRGB) == VK_IMAGE_ASPECT_COLOR_BIT);
	}

	SECTION("depth and depth-stencil formats")
	{
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_D32_SFLOAT) == VK_IMAGE_ASPECT_DEPTH_BIT);
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_D32_SFLOAT_S8_UINT) == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
	}

	SECTION("an unhandled format falls back to colour, as the warning claims")
	{
		CHECK(Cv::GetAspectFlagsFromFormat(VK_FORMAT_R16G16B16A16_SFLOAT) == VK_IMAGE_ASPECT_COLOR_BIT);
	}
}


// Regression guard over the whole public Format enum, so adding an enumerator
// without extending the aspect switch fails here rather than silently
// producing a wrong barrier. DEPTH24_STENCIL8 is why this exists: it maps to
// VK_FORMAT_D24_UNORM_S8_UINT, which the switch originally missed, so it fell
// to the default branch and reported COLOR for a depth-stencil image.
TEST_CASE("Convert: every public Format yields a sensible aspect", "[convert][aspect]")
{
	CHECK(Cv::GetAspectFlagsFromFormat(Cv::ToVkFormat(Format::RGBA8_UNORM)) == VK_IMAGE_ASPECT_COLOR_BIT);
	CHECK(Cv::GetAspectFlagsFromFormat(Cv::ToVkFormat(Format::RGBA8_SRGB)) == VK_IMAGE_ASPECT_COLOR_BIT);
	CHECK(Cv::GetAspectFlagsFromFormat(Cv::ToVkFormat(Format::DEPTH32_SFLOAT)) == VK_IMAGE_ASPECT_DEPTH_BIT);
	CHECK(Cv::GetAspectFlagsFromFormat(Cv::ToVkFormat(Format::DEPTH24_STENCIL8)) == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
}


TEST_CASE("Convert: MakeImageData translates a specification", "[convert][image]")
{
	ImageSpecification spec;
	spec.Name = "TestImage";
	spec.Width = 1280;
	spec.Height = 720;
	spec.MipLevels = 4;
	spec.Format = Format::RGBA8_SRGB;
	spec.Usage = ImageUsageFlags::COLOR_ATTACHMENT | ImageUsageFlags::SAMPLED;
	spec.Tiling = ImageTiling::LINEAR;
	spec.MemUsage = MemoryUsage::GPU_ONLY;

	const VK::VulkanImageData data = Cv::MakeImageData(spec);

	SECTION("dimensions and format carry over")
	{
		CHECK(data.Width == 1280u);
		CHECK(data.Height == 720u);
		CHECK(data.MipLevels == 4);
		CHECK(data.Format == VK_FORMAT_R8G8B8A8_SRGB);
		CHECK(data.Tiling == VK_IMAGE_TILING_LINEAR);
	}

	SECTION("layout starts undefined")
	{
		CHECK(data.Layout == VK_IMAGE_LAYOUT_UNDEFINED);
	}

	SECTION("TRANSFER_SRC is force-added on top of the requested usage")
	{
		CHECK((data.Usage & ImageUsageFlags::COLOR_ATTACHMENT) == ImageUsageFlags::COLOR_ATTACHMENT);
		CHECK((data.Usage & ImageUsageFlags::SAMPLED) == ImageUsageFlags::SAMPLED);
		CHECK((data.Usage & ImageUsageFlags::TRANSFER_SRC) == ImageUsageFlags::TRANSFER_SRC);
	}

	SECTION("it creates nothing - the Vulkan object fields stay null")
	{
		CHECK(data.Image == VK_NULL_HANDLE);
		CHECK(data.ImageView == VK_NULL_HANDLE);
		CHECK(data.Allocation == VK_NULL_HANDLE);
	}

	SECTION("queue ownership starts unknown rather than uninitialised")
	{
		CHECK(data.LastOwner == VK::QueueOwner::UNKNOWN);
		CHECK(data.CurrentOwner == VK::QueueOwner::UNKNOWN);
		CHECK(data.NextOwner == VK::QueueOwner::UNKNOWN);
	}

	SECTION("MemUsage is not represented in VulkanImageData and is dropped")
	{
		ImageSpecification other = spec;
		other.MemUsage = MemoryUsage::CPU_TO_GPU;
		const VK::VulkanImageData otherData = Cv::MakeImageData(other);

		CHECK(otherData.Format == data.Format);
		CHECK(otherData.Width == data.Width);
		// Nothing in the produced struct reflects the change.
	}
}


TEST_CASE("Convert: MakeBufferData translates a specification", "[convert][buffer]")
{
	BufferSpecification spec;
	spec.Name = "TestBuffer";
	spec.Size = 4096;
	spec.Usage = BufferUsageFlags::VERTEX_BUFFER | BufferUsageFlags::STORAGE_BUFFER;
	spec.MemUsage = MemoryUsage::GPU_ONLY;

	const VK::VulkanBufferData data = Cv::MakeBufferData(spec);

	SECTION("size carries over")
	{
		CHECK(data.Size == 4096u);
	}

	SECTION("usage carries over verbatim - nothing is force-added")
	{
		// Unlike MakeImageData, which force-adds TRANSFER_SRC, the buffer
		// path passes the requested usage through untouched. Callers that
		// need extra flags (the bindless vertex buffer) OR them in themselves.
		CHECK(data.Usage == spec.Usage);
		CHECK((data.Usage & BufferUsageFlags::TRANSFER_SRC) == BufferUsageFlags::NONE);
		CHECK((data.Usage & BufferUsageFlags::TRANSFER_DST) == BufferUsageFlags::NONE);
	}

	SECTION("offset starts at zero")
	{
		CHECK(data.Offset == 0u);
	}

	SECTION("it creates nothing - the Vulkan object fields stay null")
	{
		CHECK(data.Buffer == VK_NULL_HANDLE);
		CHECK(data.Allocation == VK_NULL_HANDLE);
		CHECK(data.AllocationInfo.size == 0u);
		CHECK(data.AllocationInfo.pMappedData == nullptr);
	}

	SECTION("queue ownership starts unknown rather than uninitialised")
	{
		// VulkanResourceManager relies on these defaults instead of assigning
		// them by hand after every CreateBuffer call.
		CHECK(data.LastOwner == VK::QueueOwner::UNKNOWN);
		CHECK(data.CurrentOwner == VK::QueueOwner::UNKNOWN);
		CHECK(data.NextOwner == VK::QueueOwner::UNKNOWN);
	}

	SECTION("a freshly translated buffer is ready")
	{
		CHECK(data.IsReady);
	}

	SECTION("Name and MemUsage are not represented in VulkanBufferData")
	{
		// Name is consumed at debug-naming time by the caller; MemUsage is
		// passed to Creators::CreateBuffer separately. Neither reaches the struct.
		BufferSpecification other = spec;
		other.Name = "DifferentName";
		other.MemUsage = MemoryUsage::CPU_TO_GPU;
		const VK::VulkanBufferData otherData = Cv::MakeBufferData(other);

		CHECK(otherData.Size == data.Size);
		CHECK(otherData.Usage == data.Usage);
		// Nothing in the produced struct reflects either change.
	}
}
