#pragma once
#include "Substrate/Exceptions.h"

#include <cstddef>
#include <cstdint>

namespace Substrate::Utility {

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


	/// <summary>
	/// Returns the base-2 logarithm of the given value. (Rounded down)
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	constexpr uint8_t Log2(uint64_t value)
	{
		if (value == 0)
			throw std::invalid_argument("Log2 is undefined for value 0");
		uint8_t log = 0;
		while (value >>= 1)
		{
			++log;
		}
		return log;
	}

	/// <summary>
	/// Returns the base-2 logarithm of the given value. Value is rounded up to next power of two (if it isnt alreay a power of two)
	/// </summary>
	/// <param name="value"></param>
	/// <returns></returns>
	constexpr uint8_t Log2Up(uint64_t value)
	{
		if (value == 0)
			throw std::invalid_argument("Log2 is undefined for value 0");
		
		if(IsPowerOfTwo(value))
			return Log2(value);

		return Log2(value) +1;
	}
}
