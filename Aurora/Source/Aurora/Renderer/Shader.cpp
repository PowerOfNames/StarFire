#include "Aurora/Renderer/Shader.h"
#include "Aurora/Utility/FileIO.h"
#include "Aurora/AppSettings.h"

#include <shaderc/shaderc.hpp>
#include <spirv_reflect.h>

#include <fstream>

namespace Aurora::VK {

	namespace Utils {
		static constexpr VkShaderStageFlagBits StringStageToVkStage(std::string_view stageString)
		{
			if (stageString == "vertex")
				return VK_SHADER_STAGE_VERTEX_BIT;
			if (stageString == "fragment")
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			if (stageString == "geometry")
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			if (stageString == "compute")
				return VK_SHADER_STAGE_COMPUTE_BIT;

			AURORA_ERROR("Unknown stageString {}", stageString);
			return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		}

		static constexpr std::string StageToCacheExtension(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return ".vert.spv";
				case VK_SHADER_STAGE_FRAGMENT_BIT: return ".frag.spv";
				default: return "";
			}
		}

		static constexpr shaderc_shader_kind StageToShaderC(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_glsl_vertex_shader;
				case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_glsl_fragment_shader;
				default: break;
			}

			AURORA_ASSERT(false, "Shaderstage not defined");
			return (shaderc_shader_kind)0; //resolves as vertex shader
		}
	}

	Ref<Shader> Shader::Create(std::string_view name, const std::filesystem::path& shaderPath)
	{
		auto shader = CreateRef<Shader>(name, shaderPath);

		if (!shader->IsValid())
			return nullptr;

		return shader;
	}
	
	Shader::Shader(std::string_view name, const std::filesystem::path& shaderPath)
		: m_DebugName(name)
	{
		std::string source = Aurora::Utils::IO::ReadFileToString(shaderPath);
		if (source.empty())
			return;

		std::unordered_map<VkShaderStageFlagBits, std::string> sources = PreProcess(source);

		auto binaries = Compile(sources);
		if (binaries.empty())
			return;

		m_IsValid = true;
		AURORA_INFO("Successfully loaded and compiled shader {}", name);
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> Shader::PreProcess(const std::string& source)
	{
		std::unordered_map<VkShaderStageFlagBits, std::string> sources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			// finds end of line AFTER the token
			// sets the begin of the 'type' to one after '#type'
			// cuts the type from the string

			size_t eol = source.find_first_of("\r\n", pos);
			AURORA_ASSERT(eol != std::string::npos, "Syntax Error!");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			AURORA_ASSERT(Utils::StringStageToVkStage(type), "Invalid shader type specified!");

			// finds the beginning of the shader string
			// sets position of the next '#type' token
			// cuts the shader code up until the next token or the end of the whole file
			size_t nextLine = source.find_first_not_of("\r\n", eol);
			AURORA_ASSERT(nextLine != std::string::npos, "Syntax Error!");
			pos = source.find(typeToken, nextLine);

			sources[Utils::StringStageToVkStage(type)] = (pos == std::string::npos) ? source.substr(nextLine) : source.substr(nextLine, pos - nextLine);
		}
		return sources;
	}

	std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> Shader::Compile(const std::unordered_map<VkShaderStageFlagBits, std::string>& sources, bool forceRecompile/* = false*/)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetTargetSpirv(shaderc_spirv_version_1_6);

		const bool optimize = false;

#ifdef AURORA_DEBUG_MODE
		options.SetOptimizationLevel(shaderc_optimization_level_zero);
#else
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
		std::filesystem::path cacheDirectory = AppSettings::Instance().ShaderCacheDirectory();

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> binaries;

		auto& shaderData = binaries;
		shaderData.clear();
		for (auto&& [stage, code] : sources)
		{
			std::filesystem::path cachedPath = (cacheDirectory / m_DebugName).concat(Utils::StageToCacheExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open() && !forceRecompile) // happens if cache path exists
			{
				in.seekg(0, std::ios::end);				// places pointer to end of file
				auto size = in.tellg();					// position of pointer equals size
				in.seekg(0, std::ios::beg);				// places pointer back to start

				auto& data = shaderData[stage];			// gets a vector of uint32_t
				data.resize(size / sizeof(uint32_t));	// sets the size of the vector
				in.read((char*)data.data(), size);		// reads in and writes to data I guess
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(code, Utils::StageToShaderC(stage), m_DebugName.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AURORA_ERROR(module.GetErrorMessage());
					{
						return {};
					}
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary | std::ofstream::trunc);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
		return binaries;
	}

	DescriptorSetLayout Shader::Reflect(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binaries)
	{
		DescriptorSetLayout shaderLayout{};


		return shaderLayout;
	}

}