#pragma once
#include "Aurora/RenderID.h"

#include <string>

namespace Aurora {

	namespace Tags {
		struct Mesh {};
		struct Shader {};
		struct Texture {};
	}

	template <typename T>
	struct AssetHandle
	{
		uint32_t Index;
		RenderID ID;
		std::string DebugName;

		constexpr const bool operator==(const AssetHandle& other) const
		{
			return other.Index == Index && other.ID == ID;
		}
		constexpr const bool operator!=(const AssetHandle& other) const
		{
			return !(*this == other);
		}
	};

	using MeshAssetHandle = AssetHandle<Tags::Mesh>;
	using ShaderAssetHandle = AssetHandle<Tags::Shader>;
	using TextureAssetHandle = AssetHandle<Tags::Texture>;

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
			return MeshAssetHandle{ UINT32_MAX, 0, "DefaultMesh" };
		}
	};

	template<>
	struct AssetTraits<Tags::Shader>
	{
		static ShaderAssetHandle GetFallback()
		{
			return ShaderAssetHandle{ UINT32_MAX, 0, "DefaultShader" };
		}
	};

	template<>
	struct AssetTraits<Tags::Texture>
	{
		static TextureAssetHandle GetFallback()
		{
			return TextureAssetHandle{ UINT32_MAX, 0, "DefaultTexture" };
		}
	};

}

namespace std {

	template<typename T>
	struct hash<Aurora::AssetHandle<T>>
	{
		std::size_t operator()(const Aurora::AssetHandle<T>& handle) const
		{
			return hash<uint64_t>()((uint64_t)handle.ID);
		}
	};
}