#include "Aurora/AppSettings.h"
#include "Aurora/Core/Core.h"


namespace Aurora {

	void AppSettings::SetRootPath(const std::filesystem::path& rootPath)
	{
		if (!std::filesystem::exists(rootPath))
		{
			AURORA_ERROR("Root path \"{}\" does not exists!", rootPath.string());
			return;
		}
		m_RootDir = rootPath;
		
		auto assetPath = rootPath / "Assets";
		if (!std::filesystem::exists(assetPath))
		{
			AURORA_WARN("Asset path \"{}\" does not exists! Please set manually using SetAssetPath", assetPath.string());
			return;
		}
		SetAssetPath(assetPath);
	}

	void AppSettings::SetAssetPath(const std::filesystem::path& assetPath)
	{
		if (!std::filesystem::exists(assetPath))
		{
			AURORA_ERROR("Asset path \"{}\" does not exists!", assetPath.string());
			return;
		}
		m_AssetDir = assetPath;

		SetShaderPath(m_AssetDir / "Shaders");
		SetTexturePath(m_AssetDir / "Textures");
		SetCacheOrCreateCacheDirectoryPath(m_RootDir / "Cache");
	}

	void AppSettings::SetShaderPath(const std::filesystem::path& shaderPath)
	{
		if (!std::filesystem::exists(shaderPath))
		{
			AURORA_ERROR("Shader path \"{}\" does not exists! Please check if the path is valid.", shaderPath.string());
			return;
		}
		m_ShaderDir = shaderPath;
	}

	void AppSettings::SetTexturePath(const std::filesystem::path& texturePath)
	{
		if (!std::filesystem::exists(texturePath))
		{
			AURORA_ERROR("Texture path \"{}\" does not exists!", texturePath.string());
			return;
		}
		m_TextureDir = texturePath;
	}

	void AppSettings::SetCacheOrCreateCacheDirectoryPath(const std::filesystem::path& cachePath)
	{
		if (!std::filesystem::exists(cachePath))
		{
			AURORA_WARN("Cache path \"{0}\" does not exists! Creating new cache directory under \"{0}\"", cachePath.string());
			std::filesystem::create_directories(cachePath);
		}
		if (!std::filesystem::exists(cachePath))
		{
			auto fallbackCache = m_RootDir / "Cache";
			AURORA_ERROR("Failed to create cache directory under \"{}\". Creating fallback directory under \"{}\"", cachePath.string(), fallbackCache.string());
			if (!std::filesystem::exists(fallbackCache))
				AURORA_ASSERT(false, "Failed to create fallback. Please check writing rights in directory.");
			m_CacheDir = fallbackCache;
		}
		else
			m_CacheDir = cachePath;

		auto shaderCache = m_CacheDir / "Shaders";
		if (!std::filesystem::exists(shaderCache))
		{
			AURORA_WARN("Shader cahce not found. Creating new under \"{}\"", shaderCache.string());
			if (!std::filesystem::exists(shaderCache))
				AURORA_ASSERT(false, "Failed to create shader cache. Please check writing rights in directory.");
		}
		m_ShaderCacheDir = shaderCache;		
	}

}