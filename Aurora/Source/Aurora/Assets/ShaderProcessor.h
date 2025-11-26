//#pragma once
//#include "Aurora/Renderer/VulkanCore.h"
//#include "Aurora/Assets/ShaderModule.h"
//
//#include <unordered_map>
//#include <string>
//#include <string_view>
//#include <expected>
//
//
//namespace Aurora {
//
//	enum class ReflectionResult
//	{
//		SUCCESS = 0,
//		REFLECT_MODULE_CREATION_FAILED,
//		ENUMERATE_INPUT_FAILED,
//		QUERY_INPUT_FAILED,
//		ENUMERATE_OUTPUT_FAILED,
//		QUERY_OUTPUT_FAILED,
//		ENUMERATE_DESCRIPTOR_SETS_FAILED,
//		QUERY_DESCRIPTOR_SETS_FAILED
//	};
//
//	class ShaderProcessor
//	{
//	public:
//		static std::unordered_map<VkShaderStageFlagBits, std::string> SplitShaderStages(const std::string& source);
//		static std::string NormalizeShaderSource_Debug(const std::string& source);
//		static std::string NormalizeShaderSource(const std::string& source);
//		static bool Compile(VkShaderStageFlagBits stage, const std::string& source, const std::string& name, std::vector<uint32_t>* outBinaries, bool forceRecompile = false);
//		//static std::expected<ShaderModule, ReflectionResult> Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& binaries);
//	};
//}
