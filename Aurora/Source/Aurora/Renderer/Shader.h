#pragma once
#include "Aurora/Core/Core.h"
#include "Substrate/RefCounted.h"
#include "Aurora/Renderer/AssetHandles.h"


#include <filesystem>
#include <string>
#include <string_view>

namespace Aurora::VK {
	

	class Shader : public Substrate::RefCounted
	{
	public:
		Shader(std::string_view name, const std::filesystem::path& path);
		~Shader() = default;
	
		inline bool IsValid() const { return m_IsValid; }

		inline const ShaderAssetHandle CreateHandle() { return ShaderAssetHandle(RenderID()); }
		static constexpr const char* StaticTypeName() { return "VulkanShader"; }
	

		static Ref<Shader> Create(std::string_view name, const std::filesystem::path& shaderPath);	
	
	private:
		ShaderAssetHandle m_Handle;
		//std::unordered_map<VkShaderStageFlagBits, ShaderModule> m_Modules;

		std::string m_DebugName = "Shader";
		bool m_IsValid = false;
	};
}
