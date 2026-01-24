#pragma once
#include "Substrate/Exceptions.h"

#include <cstddef>
#include <cstdint>

namespace Substrate {

	constexpr bool IsPowerOfTwo(uint64_t in)
	{
		return (in != 0) && ((in & (in - 1)) == 0);
	}

	/// <summary>
	/// Clamps the input value to the next highest power of two between 1 and 64 bytes.
	/// </summary>
	/// <param name="in">input in bits</param>
	/// <returns></returns>
	constexpr const uint8_t AlignUp1_MaxBytes(uint8_t bytes)
	{
		static constexpr size_t maxAlignBytes = alignof(std::max_align_t);
		if (bytes == 0)
			return 0;

		//Larger then max align
		if (bytes >= maxAlignBytes)
			return static_cast<uint8_t>(maxAlignBytes);

		bytes--;
		bytes |= bytes >> 1;
		bytes |= bytes >> 2;
		bytes |= bytes >> 4;
		return ++bytes;
	}

	constexpr const size_t AlignUpToMultipleOfMinAlignment(size_t size, size_t minAlignment)
	{
		if(!IsPowerOfTwo(minAlignment))
			throw BadAlignmentException("MinAlignment must be a power of two");
		return ((size + minAlignment) -1) & ~(minAlignment-1);		
	}
	
	//OBSOLETE: This is unnecessary, as I found out that sizeof(T) is already aligned to alignof(T) by the compiler.
	//template<typename T>
	//constexpr const size_t AlignUpToNextMaxAlignMulitple()
	//{
	//	static constexpr size_t size = sizeof(T);
	//	static constexpr size_t alignment = alignof(T);
	//	const size_t remainder = size % alignment;
	//	if (remainder == 0)
	//		return size;

	//	return size + (alignment - remainder);
	//}
}
