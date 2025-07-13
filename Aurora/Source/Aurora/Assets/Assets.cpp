#include "Aurora/Assets/Assets.h"
#include "Aurora/Assets/ShaderLoader.h"
#include "Aurora/AppSettings.h"

#include <filesystem>

namespace Aurora::Assets {
	
	ShaderAssetHandle LoadShader(std::string_view name)
	{
		return ShaderLoader::Load(name, AppSettings::Instance().ShaderDirectory());
	}
}