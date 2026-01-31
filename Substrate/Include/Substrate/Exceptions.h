#pragma once

#include <stdexcept>

namespace Substrate {

	struct HandleBitsOverflowException : public std::runtime_error
	{
		HandleBitsOverflowException(const char* message)
			: std::runtime_error(message)
		{
		}
	};

	struct AllocatorOutOfMemoryException : public std::runtime_error
	{
		AllocatorOutOfMemoryException(const char* message)
			: std::runtime_error(message)
		{
		}
	};

	struct ArrayIndexOutOfBoundsException : public std::runtime_error
	{
		ArrayIndexOutOfBoundsException(const char* message)
			: std::runtime_error(message)
		{
		}
	};

	struct BadAlignmentException : public std::runtime_error
	{
		BadAlignmentException(const char* message)
			: std::runtime_error(message)
		{
		}
	};
}
