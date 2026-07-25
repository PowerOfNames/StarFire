#pragma once


// Start: From https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
//			with changes for cleaner API usage

// (C) Copyright 2015 Just Software Solutions Ltd
//
// Distributed under the Boost Software License, Version 1.0.
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or
// organization obtaining a copy of the software and accompanying
// documentation covered by this license (the "Software") to use,
// reproduce, display, distribute, execute, and transmit the
// Software, and to prepare derivative works of the Software, and
// to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire
// statement, including the above license grant, this restriction
// and the following disclaimer, must be included in all copies
// of the Software, in whole or in part, and all derivative works
// of the Software, unless such copies or derivative works are
// solely in the form of machine-executable object code generated
// by a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE
// LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include<type_traits>

namespace Substrate {

	using BitField8 = uint8_t;
	using BitField16 = uint16_t;
	using BitField32 = uint32_t;
	using BitField64 = uint64_t;

	/// Utility macro to define bit values
#define BIT(x) (1 << x)

	/// Macro to enable bitwise operators for a specific enum class. This also imports the operators into the current namespace.		
#define SST_ENABLE_BIT_OPS(Name)\
	template<> struct enable_bitmask_operators<Name> { static constexpr bool enable = true; };

#define SST_MAKE_BIT_OPS_VISIBLE\
	using Substrate::operator|;	\
	using Substrate::operator&;	\
	using Substrate::operator^;	\
	using Substrate::operator~;	\
	using Substrate::operator|=;\
	using Substrate::operator&=;\
	using Substrate::operator^=;
	

	//From https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html with adjustments (constexpr)

	template<typename T>
	struct enable_bitmask_operators {
	static constexpr bool enable = false;
	};

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E>::type
	operator|(E lhs, E rhs) {
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(
		static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
	}

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E>::type
	operator&(E lhs, E rhs) {
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(
		static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
	}

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E>::type
	operator^(E lhs, E rhs) {
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(
		static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
	}

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E>::type
	operator~(E lhs) {
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(
		~static_cast<underlying>(lhs));
	}

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E&>::type
	operator|=(E& lhs, E rhs) {
	lhs = lhs | rhs;
	return lhs;
	}

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E&>::type
	operator&=(E& lhs, E rhs) {
	lhs = lhs & rhs;
	return lhs;
	}

	template<typename E>
	constexpr std::enable_if<enable_bitmask_operators<E>::enable, E&>::type
	operator^=(E& lhs, E rhs) {
	lhs = lhs ^ rhs;
	return lhs;
	}

	//Added:
	template<typename E>
	constexpr bool FieldHasFlag(E field, E flag)
	{
	static_assert(enable_bitmask_operators<E>::enable, "hasFlag requires a bitfield-operator enabled bitfield");
	using underlying = std::underlying_type_t<E>;
	return (static_cast<underlying>(field) & static_cast<underlying>(flag)) == static_cast<underlying>(flag);
	}
}
