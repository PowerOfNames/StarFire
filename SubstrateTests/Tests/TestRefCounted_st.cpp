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

	void DecRefPublic()
	{
		DecRef();
	}
};


TEST_CASE("RefCounted testing (single threaded) - Creation, Destruction", "[RefCounted][CstrDstr_st]")
{	
	TestClass::s_DestructorCalled = 0;
	{
		Ref<TestClass> testInstance = CreateRef<TestClass>();
		REQUIRE(testInstance->GetRefCount() == 1);
	}
	REQUIRE(TestClass::s_DestructorCalled == 1);
}

TEST_CASE("RefCounted testing (single threaded) - Copy Construction", "[RefCounted][CpyCstr_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	Ref<TestClass> instanceTwo = instanceOne;
	REQUIRE(instanceOne->GetRefCount() == 2);
	REQUIRE(instanceTwo->GetRefCount() == 2);
}

TEST_CASE("RefCounted testing (single threaded) - Move Construction", "[RefCounted][MvCstr_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);
	Ref<TestClass> instanceTwo = std::move(instanceOne);
	REQUIRE(instanceOne == nullptr);
	REQUIRE(instanceTwo->GetRefCount() == 1);
	REQUIRE(TestClass::s_DestructorCalled == 0);
}

TEST_CASE("RefCounted testing (single threaded) - Self Copy/Move", "[RefCounted][SlfCpyMv_st]")
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

TEST_CASE("RefCounted testing (single threaded) - Copy Assignment", "[RefCounted][CpyAsgn_st]")
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

TEST_CASE("RefCounted testing (single threaded) - Move Assignment", "[RefCounted][MvAsgn_st]")
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

TEST_CASE("RefCounted testing (single threaded) - Release", "[RefCounted][Release_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	REQUIRE(instanceOne->GetRefCount() == 1);
	TestClass* testRef = instanceOne.Release();
	REQUIRE(instanceOne.Get() == nullptr);
	REQUIRE(TestClass::s_DestructorCalled == 0);
	testRef->DecRefPublic();
	REQUIRE(TestClass::s_DestructorCalled == 1);
}

TEST_CASE("RefCounted testing (single threaded) - Operators", "[RefCounted][Ops_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();
	Ref<TestClass> instanceTwo = CreateRef<TestClass>();
	
	//Equality
	REQUIRE((instanceOne == instanceOne) == true);
	
	//Inequality
	REQUIRE((instanceOne != instanceTwo) == true);
}

TEST_CASE("RefCounted testing (single threaded) - nullptr", "[RefCounted][NullPtr_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = nullptr;
	Ref<TestClass> instanceTwo = CreateRef<TestClass>();

	//Equality
	REQUIRE((instanceOne.Get() == nullptr) == true);

	// Operator
	REQUIRE((instanceOne == nullptr) == true);

	Ref<TestClass> instanceThree = instanceTwo;
	REQUIRE(instanceThree->GetRefCount() == 2);
	
	//Assignment
	instanceTwo = nullptr;
	REQUIRE(instanceThree->GetRefCount() == 1);
	REQUIRE(TestClass::s_DestructorCalled == 0);
	REQUIRE((instanceTwo.Get() == nullptr) == true);
	REQUIRE((instanceTwo == nullptr) == true);

	instanceThree = nullptr;
	REQUIRE(TestClass::s_DestructorCalled == 1);
}

TEST_CASE("RefCounted testing (single threaded) - if-check", "[RefCounted][if_st]")
{
	TestClass::s_DestructorCalled = 0;

	Ref<TestClass> instanceOne = CreateRef<TestClass>();

	//true check
	REQUIRE(static_cast<bool>(instanceOne) == true);

	//Might be redundant
	if (instanceOne)
		REQUIRE(true);
	else
		REQUIRE(false);

	instanceOne = nullptr;
	//false check
	REQUIRE(static_cast<bool>(instanceOne) == false);

	//Might be redundant
	if (!instanceOne)
		REQUIRE(true);
	else
		REQUIRE(false);
}


// --- CreateRefFromThis -------------------------------------------------------
// CreateRefFromThis is protected, so these need a public shim the same way
// TestClass exposes DecRefPublic() for DecRef.

class SelfRefClass : public Substrate::RefCounted
{
public:
	inline static int s_DestructorCalled = 0;
	SelfRefClass() = default;
	virtual ~SelfRefClass()
	{
		s_DestructorCalled++;
	};

	Ref<SelfRefClass> SelfRef()
	{
		return CreateRefFromThis<SelfRefClass>();
	}
};

// The consumer shape that surfaced the missing AddRef: a scope-local object that
// copies the returned Ref into a member. Mirrors VulkanSubmissionScheduler holding
// a Ref<VulkanContext> built from CreateRefFromThis.
class SelfRefConsumer
{
public:
	explicit SelfRefConsumer(Ref<SelfRefClass> owner) : m_Owner(owner) {}
private:
	Ref<SelfRefClass> m_Owner;
};

class SelfRefBase : public Substrate::RefCounted
{
public:
	inline static int s_DestructorCalled = 0;
	SelfRefBase() = default;
	virtual ~SelfRefBase()
	{
		s_DestructorCalled++;
	};
};

class SelfRefDerived : public SelfRefBase
{
public:
	SelfRefDerived() = default;

	Ref<SelfRefDerived> SelfRef()
	{
		return CreateRefFromThis<SelfRefDerived>();
	}
};


TEST_CASE("RefCounted testing (single threaded) - CreateRefFromThis acquires a count", "[RefCounted][SelfRef_st]")
{
	SelfRefClass::s_DestructorCalled = 0;

	Ref<SelfRefClass> owner = CreateRef<SelfRefClass>();
	REQUIRE(owner->GetRefCount() == 1);
	{
		Ref<SelfRefClass> self = owner->SelfRef();
		REQUIRE(self.Get() == owner.Get());
		REQUIRE(owner->GetRefCount() == 2);
	}
	// Back to the original count, with the subject still alive: CreateRefFromThis
	// hands out a reference it acquired, not one it borrowed from its owner.
	REQUIRE(SelfRefClass::s_DestructorCalled == 0);
	REQUIRE(owner->GetRefCount() == 1);
}

TEST_CASE("RefCounted testing (single threaded) - CreateRefFromThis into a member", "[RefCounted][SelfRef_st]")
{
	SelfRefClass::s_DestructorCalled = 0;

	Ref<SelfRefClass> owner = CreateRef<SelfRefClass>();
	REQUIRE(owner->GetRefCount() == 1);
	{
		SelfRefConsumer consumer(owner->SelfRef());
		REQUIRE(owner->GetRefCount() == 2);
	}
	// Without the AddRef inside CreateRefFromThis this net-decrements: the returned
	// Ref and the consumer's member both release a single acquisition, so the object
	// is destroyed here while `owner` still holds it. Assert the destructor first,
	// or the count read below is itself a use-after-free.
	REQUIRE(SelfRefClass::s_DestructorCalled == 0);
	REQUIRE(owner->GetRefCount() == 1);
}

TEST_CASE("RefCounted testing (single threaded) - CreateRefFromThis does not drift", "[RefCounted][SelfRef_st]")
{
	SelfRefClass::s_DestructorCalled = 0;

	Ref<SelfRefClass> owner = CreateRef<SelfRefClass>();
	for (int i = 0; i < 100; i++)
	{
		SelfRefConsumer consumer(owner->SelfRef());
	}

	// Neither leaks nor over-releases across repeated use.
	REQUIRE(SelfRefClass::s_DestructorCalled == 0);
	REQUIRE(owner->GetRefCount() == 1);
}

TEST_CASE("RefCounted testing (single threaded) - CreateRefFromThis upcast counts once", "[RefCounted][SelfRef_st]")
{
	SelfRefBase::s_DestructorCalled = 0;

	Ref<SelfRefDerived> derived = CreateRef<SelfRefDerived>();
	REQUIRE(derived->GetRefCount() == 1);
	{
		Ref<SelfRefBase> base = derived->SelfRef();
		REQUIRE(base.Get() == derived.Get());
		REQUIRE(derived->GetRefCount() == 2);	// once for the upcast, not twice
	}
	REQUIRE(SelfRefBase::s_DestructorCalled == 0);
	REQUIRE(derived->GetRefCount() == 1);
}

TEST_CASE("RefCounted testing (single threaded) - CreateRefFromThis outliving its owner", "[RefCounted][SelfRef_st]")
{
	SelfRefClass::s_DestructorCalled = 0;

	Ref<SelfRefClass> self = nullptr;
	{
		Ref<SelfRefClass> owner = CreateRef<SelfRefClass>();
		self = owner->SelfRef();
		REQUIRE(owner->GetRefCount() == 2);
	}

	// The self-reference is a real owner, so it keeps the object alive on its own.
	REQUIRE(SelfRefClass::s_DestructorCalled == 0);
	REQUIRE(self->GetRefCount() == 1);

	self = nullptr;
	REQUIRE(SelfRefClass::s_DestructorCalled == 1);	// destroyed exactly once
}