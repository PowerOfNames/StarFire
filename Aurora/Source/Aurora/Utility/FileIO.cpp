#include "Aurora/Utility/FileIO.h"
#include "Aurora/Core/Core.h"

#include <fstream>

namespace Aurora::Utils::IO {

	std::string ReadFileToString(const std::filesystem::path& path)
	{
		std::string result;
		// input file stream, binary, cause we don't want to change something here. just load it
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				AURORA_ERROR("Could not read from file {}", path.string());
				in.close();
			}							
		}
		else
		{
			AURORA_ERROR("Could not open file '{}' !", path.string());
			in.close();
		}
		return result;
	}


}