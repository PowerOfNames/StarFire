#include "sfpch.h"
#include "StarFire/Core/FileSystem.h"
#include "StarFire/Core/Core.h"

#if defined(STARFIRE_PLATFORM_WINDOWS)
#include "Platform/Windows/WindowsFileHelper.h"
#else
#error "Platform not supported for FileSystem"
#endif

namespace StarFire {

	FileSystem::FileSystem()
	{
		PROFILE_FUNCTION;
		
		m_ExecutableDir = Platform::GetExecutablePath();
		STARFIRE_ASSERT(!m_ExecutableDir.empty(), "Failed to retrieve executable path!");
	}


	bool FileSystem::SetProjectDir(const std::filesystem::path& path)
	{
		PROFILE_FUNCTION;

		constexpr char sfProjExt[] = ".sfproj";

		std::error_code ec;
		if (STARFIRE_REQUIRE_FAILS(std::filesystem::exists(path, ec), "Set path {} does not exist. Error {}", path.string(), ec.message()))
			return false;
		
		const std::filesystem::path absolute = std::filesystem::canonical(path, ec);
		if (STARFIRE_REQUIRE_FAILS(!ec, "Cannot resolve project path '{}': '{}'", path.string(), ec.message()))
			return false;
		m_ProjectDir = absolute;

		bool foundSfproj = false;
		for (const auto& file : std::filesystem::directory_iterator(m_ProjectDir, ec))
		{
			if (!std::filesystem::is_regular_file(file) || file.path().extension() != sfProjExt)
				continue;
			
			if (STARFIRE_REQUIRE(!foundSfproj, "Multiple {} files found! Using the first found ({})", sfProjExt, m_ProjectFile.filename().string()))
			{
				foundSfproj = true;
				//Should already canonical/absolute due to absolute input
				m_ProjectFile = file.path();
				m_ProjectName = m_ProjectFile.stem().string();
			}
		}
		if (STARFIRE_REQUIRE_FAILS(!ec, "Error while checking directory '{}' for {} file: {}", m_ProjectDir.string(), sfProjExt, ec.message()))
			return false;
		
		if (STARFIRE_REQUIRE_FAILS(foundSfproj, "Project file does not contain required {} file!", sfProjExt))
		   return false;

		if (STARFIRE_REQUIRE_FAILS(CheckupAssetPaths(), "Some asset directories could not be resolved."))
			return false;

		if (STARFIRE_REQUIRE_FAILS(CreateCacheDirs(), "Failed to create cache directories."))
			return false;

		return true;
	}

	bool FileSystem::CheckupAssetPaths()
	{
		PROFILE_FUNCTION;

		const std::filesystem::path assetPath = m_ProjectDir / "Assets";
		if (!CheckDirectory(assetPath))
			return false;
		m_AssetDir = assetPath;

		const std::filesystem::path shaderPath = m_AssetDir / "Shaders";
		if (!CheckDirectory(shaderPath))
			return false;
		m_ShaderDir = shaderPath;

		return true;
	}

	bool FileSystem::CreateCacheDirs()
	{
		PROFILE_FUNCTION;

		//TODO: lookup proper writable memory per platform
		const std::filesystem::path cacheRoot = m_ProjectDir / "Cache";
		if (!CreateDirectory(cacheRoot))
			return false;
		m_CacheRootDir = cacheRoot;

		const std::filesystem::path shaderCache = m_CacheRootDir / "Assets"/ "Shaders";
		if (!CreateDirectory(shaderCache))
			return false;
		m_ShaderCacheDir = shaderCache;

		return true;
	}

	bool FileSystem::CheckDirectory(const std::filesystem::path& path)
	{
		PROFILE_FUNCTION;

		std::error_code ec;

		if (STARFIRE_REQUIRE_FAILS(std::filesystem::exists(path, ec), "Folder {} does not exist.", path.string()))
		{
			if (ec)
				STARFIRE_ERROR("Path existance check failed with '{}'", ec.message());

			return false;
		}
		if (STARFIRE_REQUIRE_FAILS(std::filesystem::is_directory(path, ec), "{} is not a directory.", path.string()))
		{
			if (ec)
				STARFIRE_ERROR("Path directory check failed with '{}'", ec.message());

			return false;
		}
		return true;
	}

	bool FileSystem::CreateDirectory(const std::filesystem::path& path)
	{
		PROFILE_FUNCTION;

		std::error_code ec;
		if (!std::filesystem::exists(path, ec))
		{
			if (ec)
			{
				STARFIRE_ERROR("Path existance check failed with '{}'", ec.message());
				return false;
			}
			if (STARFIRE_REQUIRE_FAILS(std::filesystem::create_directories(path, ec), "Failed to create {}: {}", path.string(), ec.message()))
				return false;
		}
		return true;
	}
}