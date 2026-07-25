#pragma once
#include "Substrate/Exceptions.h"

#include <sstream>

namespace Substrate {

	template<typename TBlockType>
	struct ArrayView
	{
		TBlockType* Data;
		size_t Count;

		TBlockType& operator[](size_t index)
		{
			if (index > Count)
			{
				std::ostringstream oss;
				oss << "Index " << index << " out of scope (" << Count << ")";
				throw ArrayIndexOutOfBoundsException(oss.str().c_str());
			}
			return Data[index];
		}
		const TBlockType& operator[](size_t index) const
		{
			if (index > Count)
			{
				std::ostringstream oss;
				oss << "Index " << index << " out of scope (" << Count << ")";
				throw ArrayIndexOutOfBoundsException(oss.str().c_str());
			}
			return Data[index];
		}
	};
}
