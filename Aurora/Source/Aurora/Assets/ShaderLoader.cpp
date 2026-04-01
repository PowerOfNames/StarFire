#include "Aurora/Assets/AssetCache.h"
#include "Aurora/Assets/ShaderLoader.h"
#include "Aurora/Profiling/Profiling.h"
#include "Aurora/Renderer/Shader.h"


namespace Aurora::Assets {

	const ShaderAssetHandle ShaderLoader::LoadImpl(std::string_view name, const std::filesystem::path& assetPath)
	{
		PROFILE_FUNCTION;

		static AssetCache<ShaderAssetHandle, VK::Shader> s_Cache;

		return s_Cache.Load(name, [&]()
			{
				//TODO: make this also work with separate shader files (.vert/.frag...) and other extensions (.hlsl)
				auto completePath = (assetPath / name).concat(".glsl");
				return VK::Shader::Create(name, completePath);
			});
	}

}