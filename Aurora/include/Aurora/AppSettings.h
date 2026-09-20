#pragma once

#include <filesystem>

namespace Aurora {

	class AppSettings
	{
	public:
		static AppSettings& Instance()
		{
			static AppSettings instance;
			return instance;
		};
		AppSettings(const AppSettings&) = delete;
		AppSettings& operator=(const AppSettings&) = delete;

		bool SetCacheRootDir(const std::filesystem::path& path);
		bool SetShaderCacheDir(const std::filesystem::path& path);
		
		inline const std::filesystem::path& GetCacheDir() const { return m_CacheRootDir; }
		inline const std::filesystem::path& GetShaderCacheDir() const { return m_ShaderCacheDir; }

	private:
		AppSettings() = default;
				
		std::filesystem::path m_CacheRootDir;		
		std::filesystem::path m_ShaderCacheDir;		
	};
}