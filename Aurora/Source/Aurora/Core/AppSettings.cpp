#include "Aurora/AppSettings.h"
#include "Aurora/Core/Core.h"


namespace Aurora {

	bool AppSettings::SetRootPath(const std::filesystem::path& rootPath)
	{
		AURORA_ASSERT(std::filesystem::exists(rootPath), "Root path \"{}\" does not exists!", rootPath.string());

		m_RootDir = rootPath;
		const std::filesystem::path assetPath = rootPath / "Assets";
		
		std::error_code ec;
		if (!std::filesystem::exists(assetPath))
		{
			if(AURORA_REQUIRE_FAILS(std::filesystem::create_directories(assetPath, ec), "Failed to create asset directory \"{}\" with error: {}.", assetPath.string(), ec.message()))
				return false;
		}
		m_AssetDir = assetPath;
		
		const std::filesystem::path shaderPath = m_AssetDir / "Shaders";
		if (!std::filesystem::exists(shaderPath))
		{
			if(AURORA_REQUIRE_FAILS(std::filesystem::create_directories(shaderPath, ec), "Failed to create shader directory \"{}\" with error: {}.", shaderPath.string(), ec.message()))
				return false;
		}
		m_ShaderDir = shaderPath;

		const std::filesystem::path texturePath = m_AssetDir / "Textures";
		if (!std::filesystem::exists(texturePath))
		{
			if(AURORA_REQUIRE_FAILS(std::filesystem::create_directories(texturePath, ec), "Failed to create texture directory \"{}\" with error: {}.", texturePath.string(), ec.message()))
				return false;
		}
		m_TextureDir = texturePath;

		const std::filesystem::path cacheRootPath = rootPath / "Cache";
		if (!std::filesystem::exists(cacheRootPath))
		{
			if(AURORA_REQUIRE_FAILS(std::filesystem::create_directories(cacheRootPath, ec), "Failed to create cache root directory \"{}\" with error: {}.", cacheRootPath.string(), ec.message()))
				return false;
		}
		m_CacheRootDir = cacheRootPath;

		const std::filesystem::path shaderCachePath = m_CacheRootDir / "Assets" / "Shaders";
		if (!std::filesystem::exists(shaderCachePath))
		{
			if(AURORA_REQUIRE_FAILS(std::filesystem::create_directories(shaderCachePath, ec), "Failed to create shader cache directory \"{}\" with error: {}.", shaderCachePath.string(), ec.message()))
				return false;
		}
		m_ShaderCacheDir = shaderCachePath;

		const std::filesystem::path textureCachePath = m_CacheRootDir / "Assets" / "Textures";
		if (!std::filesystem::exists(textureCachePath))
		{
			if (AURORA_REQUIRE_FAILS(std::filesystem::create_directories(textureCachePath, ec), "Failed to create texture cache directory \"{}\" with error: {}.", textureCachePath.string(), ec.message()))
				return false;
		}
		m_TextureCacheDir = textureCachePath;
		
		
		return true;
	}

}