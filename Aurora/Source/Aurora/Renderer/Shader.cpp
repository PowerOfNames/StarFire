#include "Aurora/Renderer/Shader.h"
#include "Aurora/Utility/FileIO.h"
#include "Aurora/AppSettings.h"
#include "Aurora/Assets/ShaderProcessor.h"


#include <fstream>

namespace Aurora::VK {
		

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

		/*std::unordered_map<VkShaderStageFlagBits, std::string> sources = ShaderProcessor::SplitShaderStages(source);

		for(const auto& [stage, src] : sources)
		{
			std::vector<uint32_t> binaries;
			if (!ShaderProcessor::Compile(stage, src, m_DebugName, &binaries))
			{
				AURORA_ERROR("Failed to compile shader stage for shader {}", name);
				return;
			}
			auto reflectResult = ShaderProcessor::Reflect(stage, binaries);
			if (!reflectResult.has_value())
			{
				AURORA_ERROR("Failed to reflect shader stage for shader {}. Reflection error code: {}", name, static_cast<int>(reflectResult.error()));
				return;
			}
			ShaderModule module = reflectResult.value();
			module.Binaries = std::move(binaries);
			module.Name = m_DebugName;
			m_Modules[stage] = std::move(module);
		}*/

		m_IsValid = true;
		AURORA_INFO("Successfully loaded and compiled shader {}", name);
	}
		

	/*void SortLayoutInfoBindings(DescriptorLayoutInfo& layoutInfo)
	{
		bool isSorted = true;
		int lastBinding = -1;
		for (uint32_t i = 0; i < layoutInfo.Bindings.size(); i++)
		{
			if (layoutInfo.Bindings[i].binding > lastBinding)
			{
				lastBinding = layoutInfo.Bindings[i].binding;
			}
			else
			{
				isSorted = false;
			}
		}
		if (!isSorted)
			layoutInfo.SortBindings();
	}*/

	/*bool CheckDescriptorHash(VkShaderStageFlagBits stage, uint32_t set, const DescriptorLayoutInfo& layoutInfo)
	{
		if (m_LayoutInfoHashes.find(stage) == m_LayoutInfoHashes.end())
		{
			AURORA_WARN("VulkanShader::CheckDescriptorHash: Shader: {} - Stage {} not contained in previous shader reflection!", m_DebugName, VulkanUtils::VKShaderStageToString(stage));
			return false;
		}
		auto& sets = m_LayoutInfoHashes.at(stage);
		if (sets.find(set) == sets.end())
		{
			AURORA_WARN("VulkanShader::CheckDescriptorHash: Shader: {}, Stage {} - Set {} not contained in previous shader reflection!", m_DebugName, VulkanUtils::VKShaderStageToString(stage), set);
			return false;
		}
		return sets.at(set) == layoutInfo.hash();
	}*/
}