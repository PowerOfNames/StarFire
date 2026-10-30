#include <catch2/catch_test_macros.hpp>

#include "Aurora/Renderer/Vulkan/Utility/VulkanConvert.h"
#include "Aurora/Renderer/Vulkan/Utility/VulkanToString.h"

// These cover Aurora::VK's *ToString helpers. They exist only for log lines,
// so what is pinned here is that every value gets a readable name and that an
// unexpected value falls back instead of producing an empty string.

using namespace Aurora;
namespace Cv = Aurora::VK::Convert;


TEST_CASE("ToString: compare ops", "[tostring][compare]")
{
	SECTION("every Vulkan compare op has its name")
	{
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_NEVER) == "Never");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_LESS) == "Less");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_EQUAL) == "Equal");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_LESS_OR_EQUAL) == "Less Or Equal");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_GREATER) == "Greater");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_NOT_EQUAL) == "Not Equal");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_GREATER_OR_EQUAL) == "Greater Or Equal");
		CHECK(VK::CompareOpToString(VK_COMPARE_OP_ALWAYS) == "Always");
	}

	SECTION("an out-of-range value falls back")
	{
		CHECK(VK::CompareOpToString(static_cast<VkCompareOp>(42)) == "Unknown Compare Op");
	}
}


// Regression guard over the whole public CompareMode enum, the same idea as the
// Format aspect guard in TestVulkanConvert.cpp: an enumerator added without a
// matching name fails here instead of logging "Unknown Compare Op".
TEST_CASE("ToString: every public CompareMode has a name", "[tostring][compare]")
{
	const CompareMode modes[] = {
		CompareMode::NEVER, CompareMode::LESS, CompareMode::EQUAL, CompareMode::LESS_OR_EQUAL,
		CompareMode::GREATER, CompareMode::NOT_EQUAL, CompareMode::GREATER_OR_EQUAL, CompareMode::ALWAYS,
	};

	for (CompareMode mode : modes)
		CHECK(VK::CompareOpToString(Cv::ToVkCompareOp(mode)) != "Unknown Compare Op");
}
