#pragma once

#include "Substrate/Exceptions.h"

#include <cstdint>


/*
	A handle is a compact representation of a resource that encodes both an index and a generation.
	The generation will be initialized with 0 and incremented each time the resource is freed.
	The generation bits' maximum value is 0xFFF...F, depending on how many bits are allocated for it.
	When the generation reaches its maximum value, the handle is considered invalid and cannot be reused. The index of an invalid generation will be blocked from being reused until the generation is reset.
	The generation can only be reset when the handle is not in use anymore, anywhere.

	The ratio between generation bits and index bits depends on the usage of the allocator it is used in. 
	As orientation, a higher generation bit count is useful for resources that are frequently created and destroyed, 
	while a higher index bit count is useful for resources that are long-lived and less frequently created and destroyed.	
*/

namespace Substrate {

	namespace HandleHelpers {

		template<typename THandleType>
		constexpr THandleType GenerateIndexMask(uint8_t bitCount)
		{
			return (bitCount >= sizeof(THandleType) * 8) ? THandleType(~0) : THandleType((1 << bitCount) - 1);
		}

		template<typename THandleType>
		constexpr THandleType GenerateGenerationMask(uint8_t bitCount)
		{
			return GenerateIndexMask<THandleType>(bitCount) << (sizeof(THandleType) * 8 - bitCount);
		}
	}

	template<typename THandleType>
	concept HandleTypeCheck = requires ()
	{
		std::is_integral_v<THandleType> &&
		std::is_unsigned_v<THandleType> &&
		std::is_floating_point_v<THandleType> == false &&
		std::is_same_v<THandleType, bool> == false &&
		std::is_same_v<THandleType, char> == false;
	};
		

	template<typename TDerivedHandle, typename THandleType, THandleType GenerationMask>
		requires HandleTypeCheck<THandleType>
	class BaseHandle
	{
	public:
		constexpr BaseHandle() : m_Handle(0) {};
		constexpr BaseHandle(THandleType index) : m_Handle(index) 
		{
			THandleType mask = static_cast<THandleType>(~GenerationMask);
			if (index > mask)
				throw HandleBitsOverflowException("Index" SST_STRINGIFY(index) "exceeds maximum value defined by IndexBits" SST_STRINGIFY(mask));

		};

		/// <summary>
		/// Increment the ID bit -> 0x0001D1A6 ID:0001 Idx:D1A6 mask -> 0xFFFF 0000 -> 0x0002D1A6
		/// </summary>
		constexpr TDerivedHandle IncrementGeneration()
		{
			if (!IsValid())
				return TDerivedHandle::FromRawType(m_Handle);
			return TDerivedHandle::FromRawType(m_Handle + static_cast<THandleType>(~GenerationMask + 1)); // ~generationMask +1 should get us the least significant bit of the generation mask 
		}


		/// <returns>Only true if the generation is not equal to the generation mask. Except when there are no generation bits, which is always a valid generation. Returns false if index == indexMask</returns>
		constexpr bool IsValid()
		{
			//If the handle's index bits are all set to 1, the handle is invalid.
			if ((m_Handle & GetIndexMask()) & INVALID_HANDLE)
				return false;

			//If the generation mask is 0, the handle is always valid.
			if(GenerationMask == 0)
				return true;

			//We return false if the generation mask is all bits set, because that means there is no valid index possible.
			if (GenerationMask == static_cast<THandleType>((~static_cast<THandleType>(0))))
				return false;

			return  (m_Handle & GenerationMask) != GenerationMask;
		}

		/// <returns>This is only true if the generations are equal AND the index! </returns>
		constexpr bool EqualsGeneration(const BaseHandle& otherHandle)
		{
			return Generation() == otherHandle.Generation();
		}

		constexpr bool EqualsRaw(THandleType other) const
		{
			return m_Handle == other;
		}

		constexpr bool Equals(const BaseHandle& otherHandle)
		{
			return EqualsRaw(otherHandle.GetRaw());
		}

		constexpr THandleType Generation() const
		{
			return (m_Handle & GenerationMask) / static_cast<THandleType>((~GenerationMask) + 1); //devide the generation by the least significant bit of the mask -> true generation value. e.g. 0xF000 is mask, 0xE000 / 0x1000 = 14, which is correct
		}

		constexpr THandleType Index() const
		{
			return m_Handle & ~GenerationMask;
		}

		constexpr THandleType GetGenerationMask() const
		{
			return GenerationMask;
		}

		constexpr THandleType GetIndexMask() const
		{
			return static_cast<THandleType>(~GenerationMask);
		}

		constexpr uint64_t GetMaxIndexValue()
		{
			return static_cast<uint64_t>(static_cast<THandleType>(~GenerationMask));
		}

		constexpr uint64_t GetMaxGenerationValue()
		{
			THandleType generation = GenerationMask;
			while ((generation & 1) != 1)
			{
				generation >>= 1;
			}
			return static_cast<uint64_t>(generation);
		}

		constexpr const THandleType GetRaw() const
		{
			return m_Handle;
		}		

		static constexpr TDerivedHandle FromRawType(THandleType handle)
		{
			return TDerivedHandle::FromRawType(handle);
		}

		/// <summary>
		/// Handle that has all index bits set to 1, generation set to max as well;
		/// </summary>
		static constexpr THandleType INVALID_HANDLE = static_cast<THandleType>(~static_cast<THandleType>(0));
	protected:
		struct InternalConstructTag {};
		constexpr BaseHandle(THandleType handle, InternalConstructTag) : m_Handle(handle) {}

	private:
		THandleType m_Handle;
	};	

	/// <summary>
	/// This struct is a helper to generate a handle type based on user defined generation and index bit counts. The sum of both bit counts must not exceed the bit size of THandleType.
	/// </summary>
	/// <typeparam name="THandleType">Must be an integral type, but not bool float or double.</typeparam>
	/// <typeparam name="GenerationBits">Number of bits dedicated to the generation.</typeparam>
	/// <typeparam name="IndexBits">Number of bits dedicated to the encoded index</typeparam>
	template<uint8_t GenerationBits, uint8_t IndexBits, typename THandleType = uint32_t>
		requires HandleTypeCheck<THandleType>
	struct GenerateHandle
	{
		static_assert(GenerationBits + IndexBits <= sizeof(THandleType) * 8, "The sum of GenerationBits and IndexBits must not exceed the bit size of THandleType.");
	
	private:
		struct Derived;

	public:
		using Type = BaseHandle<
			typename GenerateHandle<GenerationBits, IndexBits, THandleType>::Derived, 
			THandleType, 
			HandleHelpers::GenerateGenerationMask<THandleType>(GenerationBits)>;
	private:
		struct Derived : Type
		{
			static constexpr Derived FromRawType(THandleType handle)
			{
				return Derived(handle);
			}

		private:
			constexpr Derived(THandleType handle)
				: Type(handle, typename Type::InternalConstructTag{})
			{
			}
		};
	};

	template<uint8_t TGbits, uint8_t TIBits, typename THandleType = uint32_t>
		requires HandleTypeCheck<THandleType>
	using DefineHandle = typename GenerateHandle<TGbits, TIBits, THandleType>::Type;
}
