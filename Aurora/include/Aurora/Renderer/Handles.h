#pragma once
#include "Substrate/ResourceHandle.h"

#include <cstdint>
#include <limits>

namespace Aurora {

	namespace Tags {
		struct Material {};
		struct Mesh {};
		struct Shader {};
		struct Image {};
		struct Buffer {};
		struct VertexBuffer : public Buffer {};
		struct IndexBuffer : public Buffer {};
		struct Shape2D {};
		struct Texture {};
	}

	using MaterialHandle	=	Substrate::ResourceHandle<uint32_t, Tags::Material>;
	using MeshHandle		=	Substrate::ResourceHandle<uint32_t, Tags::Mesh>;
	using MeshHandle		=	Substrate::ResourceHandle<uint32_t, Tags::Mesh>;
	using ShaderHandle		=	Substrate::ResourceHandle<uint16_t, Tags::Shader>;
	using ImageHandle		=	Substrate::ResourceHandle<uint16_t, Tags::Image>;		// Used for render targets, framebuffer attachments and other renderer related
	using BufferHandle		=	Substrate::ResourceHandle<uint16_t, Tags::Buffer>;		// Used for render targets, framebuffer attachments and other renderer related
	using VertexBufferHandle = Substrate::ResourceHandle<uint16_t, Tags::VertexBuffer>;
	using IndexBufferHandle = Substrate::ResourceHandle<uint16_t, Tags::IndexBuffer>;
	using Shape2DHandle		=	Substrate::ResourceHandle<uint32_t, Tags::Shape2D>;
	using TextureHandle		=	Substrate::ResourceHandle<uint32_t, Tags::Texture>;	// Used for textures on meshes and sprites

	// ========== Tags ==========
	template<typename HandleT>
	struct GetAssetTag;

	template<>
	struct GetAssetTag<MaterialHandle> { using Type = Tags::Material; };

	template<>
	struct GetAssetTag<MeshHandle> { using Type = Tags::Mesh; };

	template<>
	struct GetAssetTag<ImageHandle> { using Type = Tags::Image; };

	template<>
	struct GetAssetTag<BufferHandle> { using Type = Tags::Buffer; };

	template<>
	struct GetAssetTag<VertexBufferHandle> { using Type = Tags::VertexBuffer; };

	template<>
	struct GetAssetTag<IndexBufferHandle> { using Type = Tags::IndexBuffer; };	

	template<>
	struct GetAssetTag<ShaderHandle> { using Type = Tags::Shader; };

	template<>
	struct GetAssetTag<Shape2DHandle> { using Type = Tags::Shape2D; };

	template<>
	struct GetAssetTag<TextureHandle> { using Type = Tags::Texture; };


	// ========== Traits ===========
	template<typename T>
	struct AssetTraits;
	
	template<>
	struct AssetTraits<Tags::Material>
	{
		static MaterialHandle GetFallback()
		{
			return MaterialHandle(std::numeric_limits<MaterialHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::Mesh>
	{
		static MeshHandle GetFallback()
		{
			return MeshHandle(std::numeric_limits<MeshHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::Image>
	{
		static ImageHandle GetFallback()
		{
			return ImageHandle(std::numeric_limits<ImageHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::Buffer>
	{
		static BufferHandle GetFallback()
		{
			return BufferHandle(std::numeric_limits<BufferHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::VertexBuffer>
	{
		static VertexBufferHandle GetFallback()
		{
			return VertexBufferHandle(std::numeric_limits<VertexBufferHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::IndexBuffer>
	{
		static IndexBufferHandle GetFallback()
		{
			return IndexBufferHandle(std::numeric_limits<IndexBufferHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::Shader>
	{
		static ShaderHandle GetFallback()
		{
			return ShaderHandle(std::numeric_limits<ShaderHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::Shape2D>
	{
		static Shape2DHandle GetFallback()
		{
			return Shape2DHandle(std::numeric_limits<Shape2DHandle>::max());
		}
	};

	template<>
	struct AssetTraits<Tags::Texture>
	{
		static TextureHandle GetFallback()
		{
			return TextureHandle(std::numeric_limits<TextureHandle>::max());
		}
	};
}