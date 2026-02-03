#pragma once
#include "Aurora/RenderID.h"
#include "Substrate/ResourceHandle.h"

#include <string>
#include <unordered_map>

namespace Aurora {

	namespace Tags {
		struct Mesh {};
		struct Shader {};
		struct Texture {};
	}

	using MeshAssetHandle = Substrate::ResourceHandle<uint16_t, Tags::Mesh>;
	using ShaderAssetHandle = Substrate::ResourceHandle<uint16_t, Tags::Shader>;
	using TextureAssetHandle = Substrate::ResourceHandle<uint16_t, Tags::Texture>;

	// ========== Tags ==========
	template<typename HandleT>
	struct GetAssetTag;

	template<>
	struct GetAssetTag<ShaderAssetHandle> { using Type = Tags::Shader; };

	template<>
	struct GetAssetTag<TextureAssetHandle> { using Type = Tags::Texture; };

	template<>
	struct GetAssetTag<MeshAssetHandle> { using Type = Tags::Mesh; };

	// ========== Traits ===========
	template<typename T>
	struct AssetTraits;
	
	template<>
	struct AssetTraits<Tags::Mesh>
	{
		static MeshAssetHandle GetFallback()
		{
			return MeshAssetHandle(UINT16_MAX);
		}
	};

	template<>
	struct AssetTraits<Tags::Shader>
	{
		static ShaderAssetHandle GetFallback()
		{
			return ShaderAssetHandle(UINT8_MAX);
		}
	};

	template<>
	struct AssetTraits<Tags::Texture>
	{
		static TextureAssetHandle GetFallback()
		{
			return TextureAssetHandle(UINT16_MAX);
		}
	};
}