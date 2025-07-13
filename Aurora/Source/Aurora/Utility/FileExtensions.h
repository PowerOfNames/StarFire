#pragma once
#include "Aurora/Utility/StringUtils.h"

#include <string>
#include <filesystem>
#include <unordered_map>

namespace Aurora {

	enum class Extension
	{
		NONE = 0,
		VERT,
		FRAG,
		GLSL,
		VERT_SPV,
		FRAG_SPV,
		SPV
	};


	namespace Utils {
				
		inline Extension FromExtensionString(const std::filesystem::path& path)
		{
			static const std::unordered_map<std::string_view, Extension> s_ExtensionMap =
			{
				{".vert",		Extension::VERT},
				{"vert",		Extension::VERT},
				{".frag",		Extension::FRAG},
				{"frag",		Extension::FRAG},
				{".glsl",		Extension::GLSL},
				{"glsl",		Extension::GLSL},
				{".vert.spv",	Extension::VERT_SPV},
				{"vert.spv",	Extension::VERT_SPV},
				{".frag.spv",	Extension::FRAG_SPV},
				{"frag.spv",	Extension::FRAG_SPV},
				{".spv",		Extension::SPV},
				{"spv",			Extension::SPV}
			};

			std::string lower = Utils::String::ToLower(path.string());
			if (auto it = s_ExtensionMap.find(lower); it != s_ExtensionMap.end())
				return it->second;

			return Extension::NONE;
		}

		/*constexpr std::string FromExtension(Extension ext)
		{
			static const std::unordered_map<Extension> s_Extensions =
			{
				{".vert",		Extension::VERT},
				{"vert",		Extension::VERT},
				{".frag",		Extension::FRAG},
				{"frag",		Extension::FRAG},
				{".glsl",		Extension::GLSL},
				{"glsl",		Extension::GLSL},
				{".vert.spv",	Extension::VERT_SPV},
				{"vert.spv",	Extension::VERT_SPV},
				{".frag.spv",	Extension::FRAG_SPV},
				{"frag.spv",	Extension::FRAG_SPV},
				{".spv",		Extension::SPV},
				{"spv",			Extension::SPV}
			};

			std::string lowerExt = Utils::String::ToLower(ext.string());
			if (auto it = s_Extensions.find(lowerExt); it != s_Extensions.end())
				return it->second;

			return Extension::NONE;
		}*/
	}
}