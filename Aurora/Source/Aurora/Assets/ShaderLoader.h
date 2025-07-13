#pragma once
#include "Aurora/Assets/AssetLoader.h"
#include "Aurora/Renderer/AssetHandles.h"

#include <filesystem>
#include <string_view>

namespace Aurora::Assets {

	class ShaderLoader;

	template<>
	struct GetAssetHandle<ShaderLoader> { using Type = ShaderAssetHandle; };

	class ShaderLoader : public AssetLoader<ShaderLoader>
	{
	public:
		static const ShaderAssetHandle LoadImpl(std::string_view name, const std::filesystem::path& assetPath);
		
	};

}