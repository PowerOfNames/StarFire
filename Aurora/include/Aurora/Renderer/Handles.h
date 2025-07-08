#pragma once
#include "Aurora/RenderID.h"

namespace Aurora {

	namespace Tags {
		struct Shader {};
		struct Texture {};
	}

	template <typename T>
	struct AssetHandle
	{
		uint32_t Index;
		RenderID ID;
	};

	using ShaderAssetHandle = AssetHandle<Tags::Shader>;
	using TextureAssetHandle = AssetHandle<Tags::Texture>;
}
