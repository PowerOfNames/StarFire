#pragma once
#if defined (STARFIRE_PLATFORM_WINDOWS)
#include "StarFire/Core/Core.h"
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace StarFire {
	namespace Platform {
	
		inline std::filesystem::path GetExecutablePath()
		{
			std::wstring buffer(512, L'\0');
			for(;;)
			{
				DWORD size = GetModuleFileNameW(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
				if (size == 0)
				{
					STARFIRE_CRITICAL("Exectuable path could not be retrieved: {}", GetLastError());
					return {};
				}
				if (size < buffer.size())
				{
					buffer.resize(size);
					return std::filesystem::path(buffer).parent_path();
				}
				if (buffer.size() >= 32768) //Windows max path length
				{
					STARFIRE_CRITICAL("Exectuable path length exceeds allowed maximum.");
					return {};
				}
				buffer.resize(buffer.size() * 2);
			}
		}
	}
}
#endif