#include "Aurora/AppSettings.h"
#include "Aurora/Core/Core.h"


namespace Aurora {
	static bool CheckDirectory(const std::filesystem::path& path)
	{
		PROFILE_FUNCTION;

		std::error_code ec;

		if (AURORA_REQUIRE_FAILS(std::filesystem::exists(path, ec), "Folder {} does not exist.", path.string()))
		{
			if (ec)
				AURORA_ERROR("Path existance check failed with '{}'", ec.message());

			return false;
		}
		if (AURORA_REQUIRE_FAILS(std::filesystem::is_directory(path, ec), "{} is not a directory.", path.string()))
		{
			if (ec)
				AURORA_ERROR("Path directory check failed with '{}'", ec.message());

			return false;
		}
		return true;
	}

	bool AppSettings::SetCacheRootDir(const std::filesystem::path& path)
	{
		PROFILE_FUNCTION;

		if (!CheckDirectory(path))
			return false;
		m_CacheRootDir = path;

		return true;
	}

	bool AppSettings::SetShaderCacheDir(const std::filesystem::path& path)
	{
		PROFILE_FUNCTION;

		if (!CheckDirectory(path))
			return false;
		m_ShaderCacheDir = path;
		
		return true;
	}
}