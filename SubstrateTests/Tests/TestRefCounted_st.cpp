#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/RefCounted.h"
#include "Substrate/RefPtr.h"

class TestClass : public Substrate::RefCounted
{
public:
	inline static int s_DestructorCalled = 0;
	TestClass() = default;
	virtual ~TestClass()
	{
		s_DestructorCalled++;
	};
};

template<typename TRefCounted>
using Ref = Substrate::RefPtr<TRefCounted>;
template<typename TRefCounted, typename ... Args>
constexpr Ref<TRefCounted> CreateRef(Args&& ... args)
{
	return Ref<TRefCounted>(new TRefCounted(std::forward<Args>(args)...));
}


TEST_CASE("RefCounted testing (single threaded) - Creation, Destruction", "[RefCountedCstrDstr_st]")
{	
	TestClass::s_DestructorCalled = 0;
	{
		Ref<TestClass> testInstance = CreateRef<TestClass>();
		REQUIRE(testInstance->GetRefCount() == 1);
	}
	REQUIRE(TestClass::s_DestructorCalled == 1);
}

TEST_CASE("RefCounted testing (single threaded) - Copy Construction", "[RefCountedCpyCstr_st]")
{
	TestClass::s_DestructorCalled = 0;


	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	Ref<TestClass> instanceTwo = instanceOne;
	REQUIRE(instanceOne->GetRefCount() == 2);
	REQUIRE(instanceTwo->GetRefCount() == 2);
}

TEST_CASE("RefCounted testing (single threaded) - Move Construction", "[RefCountedMvCstr_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);
	Ref<TestClass> instanceTwo = std::move(instanceOne);
	REQUIRE(instanceOne.Get() == nullptr);
	REQUIRE(instanceTwo->GetRefCount() == 1);
	REQUIRE(TestClass::s_DestructorCalled == 0);
}

TEST_CASE("RefCounted testing (single threaded) - Self Copy/Move", "[RefCountedSlfCpyMv_st]")
{
	TestClass::s_DestructorCalled = 0;


	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);

	//Self copy check
	instanceOne = instanceOne;
	REQUIRE(instanceOne->GetRefCount() == 1);
	REQUIRE(TestClass::s_DestructorCalled == 0);

	//Self move check
	instanceOne = std::move(instanceOne);
	REQUIRE(instanceOne->GetRefCount() == 1);
	REQUIRE(TestClass::s_DestructorCalled == 0);
}

TEST_CASE("RefCounted testing (single threaded) - Copy Assignment", "[RefCountedCpyAsgn_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);
	Ref<TestClass> instanceTwo = CreateRef<TestClass>();
	REQUIRE(instanceTwo->GetRefCount() == 1);

	//Copy assignment
	instanceOne = instanceTwo;
	REQUIRE(instanceOne->GetRefCount() == 2);
	REQUIRE(instanceTwo->GetRefCount() == 2);
	REQUIRE(TestClass::s_DestructorCalled == 1);

	//TODO: Move constructor check
	//TODO: Destruction check
}

TEST_CASE("RefCounted testing (single threaded) - Move Assignment", "[RefCountedMvAsgn_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);
	Ref<TestClass> instanceTwo = CreateRef<TestClass>();
	REQUIRE(instanceTwo->GetRefCount() == 1);

	instanceOne = std::move(instanceTwo);
	REQUIRE(instanceOne->GetRefCount() == 1);
	REQUIRE(instanceTwo.Get() == nullptr);
	REQUIRE(TestClass::s_DestructorCalled == 1);
}

TEST_CASE("RefCounted testing (single threaded) - Release", "[RefCountedRelease_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);
	TestClass* testRef = instanceOne.Release();
	REQUIRE(instanceOne.Get() == nullptr);
	REQUIRE(TestClass::s_DestructorCalled == 0);
	testRef->DecRef();
	REQUIRE(TestClass::s_DestructorCalled == 1);
}

TEST_CASE("RefCounted testing (single threaded) - Operators", "[RefCountedOps_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	Ref<TestClass> instanceTwo = CreateRef<TestClass>();
	
	//Equality
	REQUIRE((instanceOne == instanceOne) == true);
	
	//Inequality
	REQUIRE((instanceOne != instanceTwo) == true);
}
