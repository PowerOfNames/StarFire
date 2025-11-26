#pragma once

#include <filesystem>

namespace Aurora::Utils::IO {

	std::string ReadFileToString(const std::filesystem::path& path);
}
