#pragma once
#include "Aurora/Core/Core.h"
#include "Aurora/Core/RefCounted.h"
#include "Aurora/Renderer/AssetHandles.h"
#include "Aurora/Renderer/VulkanCore.h"


#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Aurora::VK {
	

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
		RenderID m_ShaderHandle;

		//std::unordered_map<VkShaderStageFlagBits, ShaderModule> m_Modules;

		std::string m_DebugName = "Shader";
		bool m_IsValid = false;
	};
}
