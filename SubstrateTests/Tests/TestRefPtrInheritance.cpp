#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/RefCounted.h"
#include "Substrate/TypeInfo.h"
#include "Substrate/RefPtr.h"


class RefCountedBase : public Substrate::RefCounted, Substrate::Base<RefCountedBase>
{
	DECLARE_TYPE(RefCountedBase, nullptr)

public:
	inline static int s_DestructorCalled = 0;
	RefCountedBase() = default;
	virtual ~RefCountedBase()
	{
		s_DestructorCalled++;
	};
};

class DerivedA : public RefCountedBase
{
	DECLARE_TYPE(DerivedA, RefCountedBase::GetStaticTypeInfo())
};

class DerivedAA : public DerivedA
{
	DECLARE_TYPE(DerivedAA, DerivedA::GetStaticTypeInfo())
};

class DerivedB : public RefCountedBase
{
	DECLARE_TYPE(DerivedB, RefCountedBase::GetStaticTypeInfo())
};


template<typename TRefCounted>
using Ref = Substrate::RefPtr<TRefCounted>;

template<typename TRefCounted, typename ... Args>
constexpr Ref<TRefCounted> CreateRef(Args&& ... args)
{
	return Ref<TRefCounted>(new TRefCounted(std::forward<Args>(args)...));
}


TEST_CASE("RefPtr inheritance testing - As", "[RefPtrInheritance_As]")
{	
	Ref<RefCountedBase> basedDerivedA = CreateRef<DerivedA>();
	Ref<RefCountedBase> basedDerivedB = CreateRef<DerivedB>();
	SECTION("Initial typeinfo check")
	{
		INFO("Static");
		REQUIRE(basedDerivedA->GetStaticTypeInfo()->Name == std::string("RefCountedBase"));
		REQUIRE(basedDerivedB->GetStaticTypeInfo()->Name == std::string("RefCountedBase"));

		INFO("Runtime");
		REQUIRE(basedDerivedA->GetTypeInfo()->Name == std::string("DerivedA"));
		REQUIRE(basedDerivedB->GetTypeInfo()->Name == std::string("DerivedB"));

		INFO("Type differences");
		REQUIRE(basedDerivedA->GetTypeInfo()->Name != basedDerivedB->GetTypeInfo()->Name);
		REQUIRE(basedDerivedA->GetTypeInfo()->ID != basedDerivedB->GetTypeInfo()->ID);
		REQUIRE(basedDerivedA->GetTypeInfo()->BaseType == basedDerivedB->GetTypeInfo()->BaseType);
	}	

	SECTION("As<> nullptr to RefPtr returns nullptr")
	{
		Ref<RefCountedBase> nullRefPtr(nullptr);
		Ref<DerivedA> asDerivedA = nullRefPtr.As<DerivedA>();

		REQUIRE(asDerivedA.Get() == nullptr);
	}

	SECTION("As<> basedDerivedA to DerivedA (Downcasting)")
	{
		Ref<DerivedA> asDerivedA = basedDerivedA.As<DerivedA>();
		REQUIRE(asDerivedA.Get() != nullptr);
		REQUIRE(asDerivedA->GetTypeInfo()->ID == basedDerivedA->GetTypeInfo()->ID);
		REQUIRE(asDerivedA->GetStaticTypeInfo()->Name == std::string("DerivedA"));
	}	

	SECTION("As<> basedDerivedA to Base (Upcasting)")
	{
		Ref<RefCountedBase> asBaseFromA = basedDerivedA.As<RefCountedBase>();

		REQUIRE(asBaseFromA.Get() != nullptr);
		REQUIRE(asBaseFromA->GetTypeInfo()->ID == basedDerivedA->GetTypeInfo()->ID);
		REQUIRE(asBaseFromA->GetStaticTypeInfo()->Name == std::string("RefCountedBase"));
	}

	SECTION("As<> basedDerivedA to DerivedB returns nullptr (Cross-casting)")
	{
		Ref<DerivedB> asDerivedB = basedDerivedA.As<DerivedB>();

		REQUIRE(asDerivedB.Get() == nullptr);
	}	

	SECTION("As<> basedDerivedA to DerivedAA (Multi-layer downcasting (invalid)")
	{
		Ref<DerivedAA> asDerivedAA = basedDerivedA.As<DerivedAA>();

		REQUIRE(asDerivedAA.Get() == nullptr);
	}

	SECTION("As<> basedDerivedAA to DerivedAA (Multi-layer downcasting (valid)")
	{
		Ref<RefCountedBase> basedDerivedAA = CreateRef<DerivedAA>();
		Ref<DerivedAA> asDerivedAA = basedDerivedAA.As<DerivedAA>();

		REQUIRE(asDerivedAA.Get() != nullptr);
		REQUIRE(asDerivedAA->GetTypeInfo()->ID == basedDerivedAA->GetTypeInfo()->ID);
		REQUIRE(asDerivedAA->GetStaticTypeInfo()->Name == std::string("DerivedAA"));
	}

	SECTION("As<> derivedAA to base (Multi-layer upcasting)")
	{
		Ref<DerivedAA> derivedAA = CreateRef<DerivedAA>();
		Ref<RefCountedBase> base = derivedAA;

		REQUIRE(base.Get() != nullptr);
		REQUIRE(base->GetTypeInfo()->ID == derivedAA->GetTypeInfo()->ID);
		REQUIRE(base->GetStaticTypeInfo()->Name == std::string("RefCountedBase"));
	}

	SECTION("As<> RefCounter")
	{
		RefCountedBase::s_DestructorCalled = 0;
		REQUIRE(basedDerivedA->GetRefCount() == 1);
		{
			Ref<DerivedA> asDerivedA = basedDerivedA.As<DerivedA>();
			REQUIRE(basedDerivedA->GetRefCount() == 2);
			REQUIRE(asDerivedA->GetRefCount() == 2);
		}
		REQUIRE(basedDerivedA->GetRefCount() == 1);
		REQUIRE(RefCountedBase::s_DestructorCalled == 0);
		RefCountedBase::s_DestructorCalled = 0;
	}
}