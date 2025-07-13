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

		inline const std::filesystem::path& ShaderCacheDirectory() const { return m_ShaderCacheDir; }
		inline const std::filesystem::path& ShaderDirectory() const { return m_ShaderDir; }


		/// <summary>
		/// Sets the path to the root directory of the application.
		/// Will look for Asset (and subdirectory) directory as well.
		/// using the assets path as root (e.g. path/to/assets/shaders/ | path/to/assets/textures/
		/// </summary>
		/// <param name="assetPath">Path to the assets directory.</param>
		/// <returns>True if path is valid, false otherwise</returns>
		void SetRootPath(const std::filesystem::path& path);
		/// <summary>
		/// Sets the path to the asset directory of the application.
		/// If path exists, also looks for Shader, Textures, etc subdirectories.
		/// using the assets path as root (e.g. path/to/assets/shaders/ | path/to/assets/textures/
		/// </summary>
		/// <param name="assetPath">Path to the assets directory.</param>
		/// <returns>True if path is valid, false otherwise</returns>
		void SetAssetPath(const std::filesystem::path& path);

		void SetShaderPath(const std::filesystem::path& path);
		void SetTexturePath(const std::filesystem::path& path);

		void SetCacheOrCreateCacheDirectoryPath(const std::filesystem::path& path);

	private:
		AppSettings() = default;

		std::filesystem::path m_RootDir = "";
		std::filesystem::path m_AssetDir = "";
		std::filesystem::path m_ShaderDir = "";
		std::filesystem::path m_TextureDir = "";
				
		std::filesystem::path m_CacheDir = "";
		std::filesystem::path m_ShaderCacheDir = "";
		
	};
}