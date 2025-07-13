#pragma once
#include "Aurora/Core/Core.h"
#include "Aurora/Core/RefCounted.h"
#include "Aurora/Renderer/VulkanCore.h"
#include "Aurora/Renderer/AssetHandles.h"


#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Aurora::VK {

	struct DescriptorSetLayout
	{
	};

	class Shader : public RefCounted<Shader>
	{
	public:
		Shader(std::string_view name, const std::filesystem::path& path);
		~Shader() = default;
	
		inline bool IsValid() const { return m_IsValid; }

		inline const ShaderAssetHandle CreateHandle() { return ShaderAssetHandle{UINT32_MAX, RenderID(), m_DebugName}; }
		static constexpr const char* StaticTypeName() { return "VulkanShader"; }
	

		static Ref<Shader> Create(std::string_view name, const std::filesystem::path& shaderPath);

	private:
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> Compile(const std::unordered_map<VkShaderStageFlagBits, std::string>& sources, bool forceRecompile = false);
		DescriptorSetLayout Reflect(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& binaries);

	private:
		std::string m_DebugName = "Shader";
		bool m_IsValid = false;
	};
}
