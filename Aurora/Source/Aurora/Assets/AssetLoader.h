#pragma once
#include "Aurora/Core/Core.h"
#include <Aurora/Renderer/AssetHandles.h>

#include <filesystem>

namespace Aurora::Assets {

	template<typename LoaderT>
	struct GetAssetHandle;	

	/*template<>
	struct GetAssetHandle<TextureLoader> { using Type = TextureAssetHandle; };

	template<>
	struct GetAssetHandle<MeshLoader> { using Type = MeshAssetHandle; };*/

	template<typename DerivedLoader>
	class AssetLoader
	{
	public:
		static typename GetAssetHandle<DerivedLoader>::Type Load(std::string_view name, const std::filesystem::path& assetPath)
		{
			return DerivedLoader::LoadImpl(name, assetPath);
		}
	};

}
