#pragma once
#include "StarFire/Core/Core.h"

#include <string>
#include <string_view>
#include <filesystem>

namespace StarFire {

	class FileSystem
	{
	public:
		FileSystem(const FileSystem&) = delete;
		FileSystem& operator=(const FileSystem&) = delete;
		FileSystem(FileSystem&&) = delete;
		FileSystem& operator=(FileSystem&&) = delete;

		~FileSystem() = default;

		bool SetProjectDir(const std::filesystem::path& path);

		// == Resource Dirs ==
		inline const std::filesystem::path& GetExecutableDirPath() const { return m_ExecutableDir; }
		inline const std::filesystem::path& GetProjectDirPath() const { return m_ProjectDir; }
		inline const std::filesystem::path& GetShaderDir() const { return m_ShaderDir; }
		
		// == Files ==
		inline const std::filesystem::path& GetProjectFilePath() const { return m_ProjectFile; }

		// == Caches ==
		inline const std::filesystem::path& GetCacheRootDir() const { return m_CacheRootDir; }
		inline const std::filesystem::path& GetShaderCacheDir() const { return m_ShaderCacheDir; }

		const std::string& GetProjectName() const { return m_ProjectName; }

		inline static FileSystem* Instance()
		{
			static FileSystem instance;
			return &instance;
		}

	private:
		FileSystem();
		bool CheckupAssetPaths();
		bool CreateCacheDirs();
		static bool CheckDirectory(const std::filesystem::path& path);
		static bool CreateDirectory(const std::filesystem::path& path);

	private:
		std::filesystem::path m_ExecutableDir;
		std::filesystem::path m_ProjectDir;
		std::filesystem::path m_ProjectFile;

		std::string m_ProjectName;

		std::filesystem::path m_AssetDir;
		std::filesystem::path m_ShaderDir;

		std::filesystem::path m_CacheRootDir;
		std::filesystem::path m_ShaderCacheDir;
	};

}
