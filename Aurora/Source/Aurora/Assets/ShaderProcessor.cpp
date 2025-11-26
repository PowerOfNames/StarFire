//#include "Aurora/Assets/ShaderProcessor.h"
//#include "Aurora/Core/Core.h"
//#include "Aurora/AppSettings.h"
//
//#include <shaderc/shaderc.hpp>
//#include <spirv_reflect.h>
//
//
//#include <filesystem>
//#include <sstream>
//#include <fstream>
//
//namespace Aurora {
//	
//
//	namespace ShaderCUtils {
//
//		static constexpr shaderc_shader_kind StageFromVK(VkShaderStageFlagBits stage)
//		{
//			switch (stage)
//			{
//				case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_glsl_vertex_shader;
//				case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_glsl_fragment_shader;
//				default: break;
//			}
//
//			AURORA_ASSERT(false, "Shaderstage not defined");
//			return (shaderc_shader_kind)0; //resolves as vertex shader
//		}
//	}
//
//	namespace SpirvUtils {
//
//		static std::string ReflectErrorToString(SpvReflectResult result)
//		{
//			switch (result)
//			{
//				case SPV_REFLECT_RESULT_SUCCESS: return "SPV_REFLECT_RESULT_Success";
//				case SPV_REFLECT_RESULT_NOT_READY: return "SPV_REFLECT_RESULT_NOT_READY";
//				case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED: return "SPV_REFLECT_RESULT__ERROR_PARSE_FAILED";
//				case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED: return "SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED";
//				case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED: return "SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED";
//				case SPV_REFLECT_RESULT_ERROR_NULL_POINTER: return "SPV_REFLECT_RESULT_ERROR_NULL_POINTER";
//				case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR: return "SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR";
//				case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH: return "SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH";
//				case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND: return "SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF: return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW: return "SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION: return "SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA: return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT";
//				case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE";
//				default: return "SPVReflect case not known!";
//			}
//		}
//
//		static std::string ReflectDecorationTypeDescriptionToString(const SpvReflectTypeDescription& type)
//		{
//			switch (type.op) {
//				case SpvOpTypeVoid: {
//					return "void";
//					break;
//				}
//				case SpvOpTypeBool: {
//					return "bool";
//					break;
//				}
//				case SpvOpTypeInt: {
//					if (type.traits.numeric.scalar.signedness)
//						return "int";
//					else
//						return "uint";
//				}
//				case SpvOpTypeFloat: {
//					switch (type.traits.numeric.scalar.width) {
//						case 32:
//							return "float";
//						case 64:
//							return "double";
//						default:
//							break;
//					}
//				}
//				case SpvOpTypeStruct: {
//					return "struct";
//				}
//				default: {
//					break;
//				}
//			}
//			return "";
//		}
//
//		static std::string ReflectDescriptorTypeToString(SpvReflectDescriptorType value) {
//			switch (value) {
//				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return "VK_DESCRIPTOR_TYPE_SAMPLER";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
//				case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR";
//			}
//			// unhandled SpvReflectDescriptorType enum value
//			return "VK_DESCRIPTOR_TYPE_???";
//		}
//
//		static std::string ReflectShaderStageToString(SpvReflectShaderStageFlagBits stage) {
//			switch (stage) {
//				case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT: return "Reflected Vertex Stage";
//				case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT: return "Reflected Fragment Stage";
//				case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT: return "Reflected Geometry Stage";
//				case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT: return "Reflected Compute Stage";
//				case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return "Reflected Tessellation Control Stage";
//				case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return "Reflected Tessellation Eva Stage";
//			}
//			return "SPV_SHADER:_STAGE_???";
//		}
//
//		VkDescriptorType IsDynamic(VkDescriptorType type, uint32_t set)
//		{
//			if (set != 1)
//				return type;
//
//			switch (type)
//			{
//				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
//				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
//				default:
//				{
//					AURORA_WARN("Type does not support DYNAMIC!");
//					return type;
//				}
//			}
//		}
//
//	}
//
//	std::unordered_map<VkShaderStageFlagBits, std::string> ShaderProcessor::SplitShaderStages(const std::string& source)
//	{
//		std::unordered_map<std::string, std::ostringstream> stageSources;
//
//		std::istringstream stream(source);
//		std::string line;
//		std::string currentStage;
//
//		while (std::getline(stream, line))
//		{
//			line.erase(0, line.find_first_not_of(" \t\r"));
//			line.erase(line.find_last_not_of(" \t\r") + 1);
//
//			//means, look for #type at start of line -> found new stage
//			if (line.rfind("#type", 0) == 0)
//			{
//				currentStage = line.substr(5);
//				currentStage.erase(0, currentStage.find_first_not_of(" \t"));
//			}
//			else if (!currentStage.empty())
//			{
//				stageSources[currentStage] << line << '\n';
//			}
//		}
//
//		std::unordered_map<VkShaderStageFlagBits, std::string> result;
//		for (auto& [stage, buffer] : stageSources)
//		{
//			result[Utils::VK::StringToShaderStage(stage)] = buffer.str();
//		}
//		return result;
//	}
//
//	std::string ShaderProcessor::NormalizeShaderSource_Debug(const std::string& source)
//	{
//		enum class State
//		{
//			NONE = 0,
//			LINE_COMMENT,
//			MULITLINE_COMMENT,
//			STRING_LITERAL,
//			PREPROCESSING_LITERAL,
//
//			CODE = NONE
//		};
//
//		std::string out;
//		State state = State::NONE;
//		size_t size = source.size();
//
//		for (size_t i = 0; i < size; i++)
//		{
//			char c = source[i];
//			char next = (i + 1) < size ? source[i + 1] : '\0';
//
//			switch (state)
//			{
//				case State::CODE:
//				{
//					if (c == '/' && next == '/')
//					{
//						state == State::LINE_COMMENT;
//						i++;
//					}
//					else if (c == '/' && next == '*')
//					{
//						state = State::MULITLINE_COMMENT;
//						i++;
//					}
//					else if (c == '"')
//					{
//						state = State::STRING_LITERAL;
//						out += c;
//					}
//					else
//						out += c;
//					break;
//				}
//				case State::LINE_COMMENT:
//				{
//					if (c == '\n')
//					{
//						state == State::CODE;
//						out += c;
//					}
//					break;
//				}
//				case State::MULITLINE_COMMENT:
//				{
//					if (c == '*' && next == '/')
//					{
//						state == State::CODE;
//						i++;
//					}
//					break;
//				}
//				case State::STRING_LITERAL:
//				{
//					out += c;
//					//quotations in strings -> skip this quota
//					if (c == '\\' && next == '"')
//					{
//						out += next;
//						i++;
//					}
//					else if (c == '"')
//					{
//						state == State::CODE;
//					}
//					break;
//				}
//			}
//		}
//
//		return out;
//	}
//	std::string ShaderProcessor::NormalizeShaderSource(const std::string& source)
//	{
//		enum class State
//		{
//			NONE = 0,
//			LINE_COMMENT,
//			MULITLINE_COMMENT,
//			PREPROCESSING_LITERAL,
//
//			CODE = NONE
//		};
//
//		std::string out;
//		State state = State::NONE;
//		size_t size = source.size();
//
//		for (size_t i = 0; i < size; i++)
//		{
//			char c = source[i];
//			char next = (i + 1) < size ? source[i + 1] : '\0';
//
//			switch (state)
//			{
//				case State::CODE:
//				{
//					if (c == '/' && next == '/')
//					{
//						state == State::LINE_COMMENT;
//						i++;
//					}
//					else if (c == '/' && next == '*')
//					{
//						state = State::MULITLINE_COMMENT;
//						i++;
//					}
//					else
//						out += c;
//					break;
//				}
//				case State::LINE_COMMENT:
//				{
//					if (c == '\n')
//					{
//						state == State::CODE;
//						out += c;
//					}
//					break;
//				}
//				case State::MULITLINE_COMMENT:
//				{
//					if (c == '*' && next == '/')
//					{
//						state == State::CODE;
//						i++;
//					}
//					break;
//				}
//			}
//		}
//
//		return out;
//	}
//	
//	bool Compile(VkShaderStageFlagBits stage, const std::string& source, const std::string& name, std::vector<uint32_t>* outBinaries, bool forceRecompile/* = false*/)
//	{
//		shaderc::Compiler compiler;
//		shaderc::CompileOptions options;
//
//		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
//		options.SetTargetSpirv(shaderc_spirv_version_1_6);
//
//#ifdef AURORA_DEBUG_MODE
//		options.SetOptimizationLevel(shaderc_optimization_level_zero);
//#else
//		options.SetOptimizationLevel(shaderc_optimization_level_performance);
//#endif
//
//		std::filesystem::path cacheDirectory = AppSettings::Instance().ShaderCacheDirectory();
//		std::filesystem::path cachedPath = (cacheDirectory / name).concat(Utils::VK::ShaderStageToCacheExtension(stage));
//
//		std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
//		if (in.is_open() && !forceRecompile)							// happens if cache path exists
//		{
//			in.seekg(0, std::ios::end);				// places pointer to end of file
//			auto size = in.tellg();					// position of pointer equals size
//			in.seekg(0, std::ios::beg);				// places pointer back to start
//
//			outBinaries->resize(size / sizeof(uint32_t));	// sets the size of the vector
//			in.read(reinterpret_cast<char*>(outBinaries->data()), size);		// reads in and writes to data I guess
//			in.close();
//		}
//		else
//		{
//			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, ShaderCUtils::StageFromVK(stage), name.c_str(), options);
//			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
//			{
//				AURORA_ERROR(module.GetErrorMessage());			
//				return false;
//			}
//
//			outBinaries->assign(module.cbegin(), module.cend());
//
//			std::ofstream out(cachedPath, std::ios::out | std::ios::binary | std::ios::trunc);
//			if (out.is_open())
//			{
//				out.write(reinterpret_cast<char*>(outBinaries->data()), outBinaries->size() * sizeof(uint32_t));
//				out.flush();
//				out.close();
//			}
//		}
//		
//		return true;
//	}
//
//
//	std::expected<ShaderModule, ReflectionResult> ShaderProcessor::Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& binaries)
//	{
//		//Reflect:
//		//Vertex shader:
//		// - Input attributes and bindings
//		// - Uniform buffers
//		// - Storage buffers
//		// - Textures
//		// - Push constants
//		// 
//		// - Input variables
//		// - Output variables
//		size_t binarySize = binaries.size() * sizeof(uint32_t);
//
//		ShaderModule module{};
//
//		SpvReflectShaderModule reflectionModule;
//		SpvReflectResult rc = spvReflectCreateShaderModule(binarySize, binaries.data(), &reflectionModule);
//		if (rc != SpvReflectResult::SPV_REFLECT_RESULT_SUCCESS)
//		{
//			AURORA_ERROR("Failed to create reflection module.");
//			return std::unexpected(ReflectionResult::REFLECT_MODULE_CREATION_FAILED);
//		}
//
//		if (stage == VK_SHADER_STAGE_VERTEX_BIT)
//		{
//			uint32_t count = 0;
//			rc = spvReflectEnumerateInputVariables(&reflectionModule, &count, nullptr);
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("InputVariable enumeration failed!");
//				return std::unexpected(ReflectionResult::ENUMERATE_INPUT_FAILED);
//			}
//
//			std::vector<SpvReflectInterfaceVariable*> inputVars(count);
//			rc = spvReflectEnumerateInputVariables(&reflectionModule, &count, inputVars.data());
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("InputVariable querying failed!");
//				return std::unexpected(ReflectionResult::QUERY_INPUT_FAILED);
//			}
//
//			count = 0;
//			rc = spvReflectEnumerateOutputVariables(&reflectionModule, &count, nullptr);
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("OutputVariable enumeration failed!");
//				return std::unexpected(ReflectionResult::ENUMERATE_OUTPUT_FAILED);
//			}
//
//			std::vector<SpvReflectInterfaceVariable*> outputVars(count);
//			rc = spvReflectEnumerateOutputVariables(&reflectionModule, &count, outputVars.data());
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("OutputVariable querying failed!");
//				return std::unexpected(ReflectionResult::QUERY_INPUT_FAILED);
//			}
//
//			module.VertexBinding.binding = 0;
//			module.VertexBinding.stride = 0;
//			module.VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//			module.VertexAttributes.reserve(inputVars.size());
//			for (size_t i = 0; i < inputVars.size(); i++)
//			{
//				const SpvReflectInterfaceVariable& reflectVar = *(inputVars[i]);
//				//ignore build-in variables
//				if (reflectVar.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
//					continue;
//				VkVertexInputAttributeDescription attributeDescription{};
//				attributeDescription.location = reflectVar.location;
//				attributeDescription.binding = module.VertexBinding.binding;
//				attributeDescription.format = static_cast<VkFormat>(reflectVar.format);
//				attributeDescription.offset = 0; //Later 
//				module.VertexAttributes.push_back(attributeDescription);
//			}
//			//sort ascending
//			std::sort(std::begin(module.VertexAttributes), std::end(module.VertexAttributes), [](const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b)
//				{
//					return a.location < b.location;
//				});
//			//calculate offset
//			for (auto& attribute : module.VertexAttributes)
//			{
//				uint32_t formatSize = Utils::VK::FormatSize(attribute.format);
//				attribute.offset = module.VertexBinding.stride;
//				module.VertexBinding.stride += formatSize;
//			}
//			//m_VertexInputDescription.Bindings.push_back(bindingDescription);
//
//			//Debug Print Input and outputs
//#ifdef AURORA_DEBUG_MODE
//			AURORA_INFO("Entry Point: {}", reflectionModule.entry_point_name);
//			AURORA_INFO("Source Language: {}", spvReflectSourceLanguage(reflectionModule.source_language));
//			AURORA_INFO("Source Language Version: {}", reflectionModule.source_language_version);
//			if (reflectionModule.source_language == SpvSourceLanguageGLSL)
//			{
//				switch (reflectionModule.shader_stage)
//				{
//					case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT: AURORA_INFO("Shader Stage 'Vertex (VS)'"); break;
//					case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT: AURORA_INFO("Shader Stage 'TesselationControl (HS)'"); break;
//					case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: AURORA_INFO("Shader Stage 'TessellationEvalation (DS)'"); break;
//					case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT: AURORA_INFO("Shader Stage 'Geometry (GS)'"); break;
//					case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT: AURORA_INFO("Shader Stage 'Fragment (FS)'"); break;
//					case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT: AURORA_INFO("Shader Stage 'Compute (CS)'"); break;
//				}
//			}
//
//			AURORA_INFO("Input Variables: ");
//			for (size_t i = 0; i < module.VertexAttributes.size(); i++)
//			{
//				SpvReflectInterfaceVariable* var = inputVars[i];
//				AURORA_INFO("layout(location = {}) in {} {}",
//					var->location,
//					SpirvUtils::ReflectDecorationTypeDescriptionToString(*var->type_description).c_str(),
//					var->name
//				);
//			}
//
//			AURORA_INFO("Out Variables: ");
//			for (size_t i = 0; i < outputVars.size(); i++)
//			{
//				SpvReflectInterfaceVariable* var = outputVars[i];
//				if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
//					continue;
//				AURORA_INFO("layout(location = {0}) out {1} {2}",
//					var->location,
//					SpirvUtils::ReflectDecorationTypeDescriptionToString(*var->type_description).c_str(),
//					var->name
//				);
//			}
//#endif
//		}
//		else
//		{
//			uint32_t count = 0;
//			rc = spvReflectEnumerateInputVariables(&reflectionModule, &count, nullptr);
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("InputVariable enumeration failed!");
//				return std::unexpected(ReflectionResult::ENUMERATE_INPUT_FAILED);
//			}
//
//			std::vector<SpvReflectInterfaceVariable*> inputVars(count);
//			rc = spvReflectEnumerateInputVariables(&reflectionModule, &count, inputVars.data());
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("InputVariable querying failed!");
//				return std::unexpected(ReflectionResult::QUERY_INPUT_FAILED);
//			}
//
//			count = 0;
//			rc = spvReflectEnumerateOutputVariables(&reflectionModule, &count, nullptr);
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("OutputVariable enumeration failed!");
//				return std::unexpected(ReflectionResult::ENUMERATE_OUTPUT_FAILED);
//			}
//
//			std::vector<SpvReflectInterfaceVariable*> outputVars(count);
//			rc = spvReflectEnumerateOutputVariables(&reflectionModule, &count, outputVars.data());
//			if (rc != SPV_REFLECT_RESULT_SUCCESS)
//			{
//				AURORA_ERROR("OutputVariable querying failed!");
//				return std::unexpected(ReflectionResult::QUERY_INPUT_FAILED);
//			}
//
//			module.Inputs.resize(inputVars.size());
//			for (size_t i = 0; i < inputVars.size(); i++)
//			{
//				const SpvReflectInterfaceVariable& reflectVar = *(inputVars[i]);
//				//ignore build-in variables
//				if (reflectVar.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
//					continue;
//				auto& input = module.Inputs[i];
//				input.Location = reflectVar.location;
//				input.FieldType = Utils::VK::FormatToFieldType(static_cast<VkFormat>(reflectVar.format));
//			}
//
//			module.Outputs.resize(outputVars.size());
//			for (size_t i = 0; i < outputVars.size(); i++)
//			{
//				const SpvReflectInterfaceVariable& reflectVar = *(outputVars[i]);
//				//ignore build-in variables
//				if (reflectVar.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
//					continue;
//				auto& output = module.Outputs[i];
//				output.Location = reflectVar.location;
//				output.FieldType = Utils::VK::FormatToFieldType(static_cast<VkFormat>(reflectVar.format));
//			}
//		}
//
//		// DesciptorSets
//		uint32_t count = 0;
//		rc = spvReflectEnumerateDescriptorSets(&reflectionModule, &count, nullptr);
//		if (rc != SPV_REFLECT_RESULT_SUCCESS)
//		{
//			AURORA_ERROR("DescriptorSets enumeration failed!");
//			return std::unexpected(ReflectionResult::ENUMERATE_DESCRIPTOR_SETS_FAILED);
//		}
//		std::vector<SpvReflectDescriptorSet*> reflSets(count);
//		rc = spvReflectEnumerateDescriptorSets(&reflectionModule, &count, reflSets.data());
//		if (rc != SPV_REFLECT_RESULT_SUCCESS)
//		{
//			AURORA_ERROR("Failed ot query reflection descriptor sets");
//			return std::unexpected(ReflectionResult::QUERY_DESCRIPTOR_SETS_FAILED);
//		}
//
//		for (size_t i = 0; i < reflSets.size(); i++)
//		{
//			const SpvReflectDescriptorSet& reflSet = *(reflSets[i]);
//
//		}
//	}
////
////	std::unordered_map<VkShaderStageFlagBits, ShaderModule> Reflect(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binaries)
////	{
////		ShaderDescriptorInfo shaderLayout{};
////
////		std::map<uint32_t, ShaderDescriptorInfo> setToLayoutInfos;
////
////		for (const auto& [stage, data] : binaries)
////		{
////			std::vector<SpvReflectDescriptorSet*> reflSets(count);
////			result = spvReflectEnumerateDescriptorSets(&module, &count, reflSets.data());
////			if (result != SPV_REFLECT_RESULT_SUCCESS)
////			{
////				AURORA_ERROR("DescriptorSets querying failed!");
////				return {};
////			}
////
////			//For every set in shaderStage
////			for (size_t i = 0; i < reflSets.size(); i++)
////			{
////				const SpvReflectDescriptorSet& reflSet = *(reflSets[i]);
////
////				// abuse operator[] -> if not existant, creates an returns new. I therefor just need to check the bindings -> add binding if not existant, or update stage flag if existant
////				DescriptorLayoutInfo& currentSetLayout = setToLayoutInfos[reflSet.set];
////				for (size_t j = 0; j < reflSet.binding_count; j++)
////				{
////					const SpvReflectDescriptorBinding& reflBinding = *(reflSet.bindings[j]);
////
////					const char* name;
////					if (VulkanUtils::IsImageBinding(static_cast<VkDescriptorType>(reflBinding.descriptor_type)))
////						name = reflBinding.name;
////					else
////						name = reflBinding.type_description->type_name;
////
////					//check if binding is already in Set
////					auto it = std::find_if(currentSetLayout.Bindings.begin(), currentSetLayout.Bindings.end(),
////						[=](const VkDescriptorSetLayoutBinding& binding) {return binding.binding == reflBinding.binding; });
////					if (it == currentSetLayout.Bindings.end())
////					{
////						//Not found -> add new binding
////						VkDescriptorSetLayoutBinding binding{};
////						binding.binding = reflBinding.binding;
////						binding.descriptorType = SpirvUtils::IsDynamic(static_cast<VkDescriptorType>(reflBinding.descriptor_type), reflSet.set);
////						binding.descriptorCount = 1;
////						for (uint32_t dim = 0; dim < reflBinding.array.dims_count; dim++)
////						{
////							binding.descriptorCount *= reflBinding.array.dims[dim];
////						}
////						binding.stageFlags |= static_cast<VkShaderStageFlagBits>(module.shader_stage);
////						if (reflBinding.set == 0)
////							binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
////
////						currentSetLayout.Bindings.push_back(binding);
////
////						Ref<ShaderResourceDescription> resource = CreateRef<ShaderResourceDescription>();
////						resource->Set = reflSet.set;
////						resource->Binding = reflBinding.binding;
////						resource->Count = binding.descriptorCount;
////						resource->Name = name;
////						resource->ResourceType = VulkanUtils::VulkanDescriptorTypeToShaderResourceType(SpirvUtils::IsDynamic(static_cast<VkDescriptorType>(reflBinding.descriptor_type), reflSet.set));
////						if (reflSet.set == 0)
////							resource->Stages = ShaderStage::ALL_STAGES;
////						else
////							resource->Stages |= SpirvUtils::ReflectShaderStageToStage(module.shader_stage);
////
////						m_ShaderResourceDescriptions[name] = std::move(resource);
////					}
////					else
////					{
////						auto index = std::distance(currentSetLayout.Bindings.begin(), it);
////						currentSetLayout.Bindings[index].stageFlags |= static_cast<VkShaderStageFlagBits>(module.shader_stage);
////
////						m_ShaderResourceDescriptions[name]->Stages |= SpirvUtils::ReflectShaderStageToStage(module.shader_stage);
////					}
////
////					SortLayoutInfoBindings(currentSetLayout);
////					m_LayoutInfoHashes[stage][reflSet.set] = currentSetLayout.hash();
////				}
////
////				if (!CheckDescriptorHash(stage, reflSet.set, setToLayoutInfos.at(reflSet.set)))
////				{
////					AURORA_WARN("Shader::Reflect: Shader: {}, Stage: {} - DescriptorSet {} does not match previous layout!", m_DebugName, VulkanUtils::VKShaderStageToString(stage), reflSet.set);
////					return false;
////				}
////			}
////
////			//Debug print descriptors
////#if AURORA_DEBUG_MODE
////			for (size_t i_sets = 0; i_sets < reflSets.size(); i_sets++)
////			{
////				SpvReflectDescriptorSet* reflSet = reflSets[i_sets];
////
////				AURORA_INFO("Set: {0}", reflSet->set);
////				AURORA_INFO("BindingCount: {0}", reflSet->binding_count);
////
////				for (uint32_t i_bindings = 0; i_bindings < reflSet->binding_count; i_bindings++)
////				{
////					AURORA_INFO("Layout(Set = {}, Binding = {}) {} {}", reflSet->bindings[i_bindings]->set, reflSet->bindings[i_bindings]->binding,
////						SpirvUtils::ReflectDescriptorTypeToString(reflSet->bindings[i_bindings]->descriptor_type).c_str(), reflSet->bindings[i_bindings]->name);
////
////
////					//Array
////					if (reflSet->bindings[i_bindings]->array.dims_count > 0)
////					{
////						for (uint32_t k = 0; k < reflSet->bindings[i_bindings]->array.dims_count; k++)
////						{
////							AURORA_INFO("Array [{0}]", reflSet->bindings[i_bindings]->array.dims[k]);
////						}
////					}
////
////					//Counter?
////					if (reflSet->bindings[i_bindings]->uav_counter_binding != nullptr)
////					{
////						AURORA_INFO("Counter: (Set={0}, Binding={1}, Name={2})",
////							reflSet->bindings[i_bindings]->uav_counter_binding->set,
////							reflSet->bindings[i_bindings]->binding,
////							reflSet->bindings[i_bindings]->name
////						);
////					}
////				}
////			}//for-end DescriptorSet-debug 	
////
////#endif
////
////			spvReflectDestroyShaderModule(&module);
////		}//for-end stages
////
////		return shaderLayout;
////	}
//
//}