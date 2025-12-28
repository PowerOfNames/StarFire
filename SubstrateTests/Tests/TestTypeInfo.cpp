#include <catch2/catch_test_macros.hpp>

#include "Substrate/TypeInfo.h"

#include <unordered_map>
#include <random>

class TestClass : public Substrate::Base<TestClass>
{
	DECLARE_TYPE(TestClass, nullptr)
};

class DerivedTestClass : public TestClass
{
	DECLARE_TYPE(DerivedTestClass, TestClass::GetStaticTypeInfo())
};

TEST_CASE("TypeInfo testing - GetStaticTypeInfo", "[TypeInfo_GetStaticTypeInfo]")
{
	const Substrate::TypeInfo* typeInfo = TestClass::GetStaticTypeInfo();
	REQUIRE(typeInfo != nullptr);
	REQUIRE(typeInfo->Name == std::string("TestClass"));
	REQUIRE(typeInfo->BaseType == nullptr);
	REQUIRE(typeInfo->ID == Substrate::HashTypeNameFNV1a("TestClass"));
}

TEST_CASE("TypeInfo testing - IDHashGeneration", "[TypeInfo_IDHashingGeneration]")
{
	SECTION("Predictability")
	{
		uint32_t hash1 = Substrate::HashTypeNameFNV1a("TestClass");
		uint32_t hash11 = Substrate::HashTypeNameFNV1a("TestClass");
		REQUIRE(hash1 == hash11);
	}

	std::unordered_map<uint32_t, std::string> generatedHashes;
	SECTION("Sequential Names")
	{
		uint32_t collisionCount = 0;
		constexpr uint32_t c_Checks = 1000000;
		constexpr uint32_t c_ExpectedCollisions = c_Checks / 10000; //0.01% collision rate expected
		for (int i = 0; i < c_Checks; ++i)
		{
			std::string typeName = "TestClass" + std::to_string(i);
			uint32_t hash = Substrate::HashTypeNameFNV1a(typeName.c_str());
			if (generatedHashes.find(hash) != generatedHashes.end())
			{
				if(generatedHashes[hash] == typeName)
					continue;

				collisionCount++;
				UNSCOPED_INFO("Collision found for type name: " << typeName << " with hash: " << hash << ", previously used by type name: " << generatedHashes[hash]);
			}
				
			generatedHashes[hash] = typeName;
		}
		INFO("Total collisions found: " << collisionCount);
		REQUIRE(collisionCount < c_ExpectedCollisions);
	}
	generatedHashes.clear();

	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<uint32_t> charDist(33, 126); //ASCII printable characters

	SECTION("Random Names")
	{
		uint32_t collisionCount = 0;
		constexpr uint32_t c_Checks = 1000000;
		constexpr uint32_t c_ExpectedCollisions = c_Checks / 10000; //0.01% collision rate expected
		for (int i = 0; i < 100000; ++i)
		{
			std::string typeName;
			uint32_t stringLen = std::uniform_int_distribution<uint32_t>(5, 20)(rng);
			for (uint32_t i = 0; i < stringLen; i++)
			{
				typeName += static_cast<char>(charDist(rng));
			}
			uint32_t hash = Substrate::HashTypeNameFNV1a(typeName.c_str());
			if (generatedHashes.find(hash) != generatedHashes.end())
			{
				if (generatedHashes[hash] == typeName)
					continue;

				collisionCount++;
				UNSCOPED_INFO("Collision found for type name: " << typeName << " with hash: " << hash << ", previously used by type name: " << generatedHashes[hash]);
			}
			generatedHashes[hash] = typeName;
		}
		INFO("Total collisions found: " << collisionCount);
		REQUIRE(collisionCount < c_ExpectedCollisions);
	}
}

TEST_CASE("TypeInfo testing - Inheritance", "[TypeInfo_Inheritance]")
{
	const Substrate::TypeInfo* typeInfo = TestClass::GetStaticTypeInfo();
	const Substrate::TypeInfo* derivedTypeInfo = DerivedTestClass::GetStaticTypeInfo();
	REQUIRE(derivedTypeInfo != nullptr);
	REQUIRE(derivedTypeInfo->Name == std::string("DerivedTestClass"));
	REQUIRE(derivedTypeInfo->BaseType == typeInfo);
	REQUIRE(derivedTypeInfo->BaseType->Name == std::string("TestClass"));
	REQUIRE(derivedTypeInfo->ID == Substrate::HashTypeNameFNV1a("DerivedTestClass"));
}