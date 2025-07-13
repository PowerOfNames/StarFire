#pragma once

#include "Aurora/Renderer/AssetHandles.h"

#include <filesystem>
#include <string_view>

namespace Aurora::Assets {
	
	
	/// <summary>
	/// Loads all shaders with that name stitching them together (name.vert + name.geom + name.frag)
	/// </summary>
	/// <param name="name">The name of the shader.</param>
	/// <returns>Returns the shader asset handle. If name wasn't found, returns handle with ID = 0 (invalid)
	/// and Index UINT23_TMAX</returns>
	ShaderAssetHandle LoadShader(std::string_view name);
}
