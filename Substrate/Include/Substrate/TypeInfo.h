#pragma once

#include <cstdint>
#include <concepts>

#define DECLARE_TYPE(type, ...) \
	public: \
    static const Substrate::TypeInfo* GetStaticTypeInfo() { return &s_##type##TypeInfo; } \
	private: \
	inline static const Substrate::TypeInfo s_##type##TypeInfo = { #type, ##__VA_ARGS__, Substrate::HashTypeNameFNV1a(#type) };

namespace Substrate {

	uint32_t constexpr HashTypeNameFNV1a(const char* typeName)
	{
		uint32_t hash = 2166136261u;
		while (*typeName)
		{
			hash ^= static_cast<uint32_t>(*typeName++);
			hash *= 16777619u;
		}
		return hash;
	}

	struct TypeInfo
	{
		const char* Name;
		const TypeInfo* BaseType;
		const uint32_t ID;
	};


	template<typename TDerived>
	concept IsDerivedWithTypeInfo = requires
	{
		{ TDerived::GetStaticTypeInfo() } -> std::same_as<const TypeInfo*>;
	};


	template<typename TDerived>
	class Base
	{
	public:
		Base()
		{
			static_assert(IsDerivedWithTypeInfo<TDerived>, "Classes derived from Substrate::Base must implement GetStaticTypeInfo via DECLARE_TYPE macro");
		}

	protected:
		virtual ~Base() = default;
	};


}
