#include "../src/Dyma.hpp"
#include "doctest.h"

using namespace dyma;

DOCTEST_TEST_CASE("NullAllocator")
{
	DOCTEST_SUBCASE("Allocate")
	{
		NullAllocator allocator;
		DOCTEST_CHECK(allocator.Allocate(0) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(1) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(2) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(3) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(4) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(8) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(16) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(32) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(64) == nullptr);
		DOCTEST_CHECK(allocator.Allocate(128) == nullptr);
	}

	DOCTEST_SUBCASE("Deallocate")
	{
		NullAllocator allocator;

		void* iPtr = nullptr;
		DOCTEST_CHECK(!allocator.Deallocate(iPtr));

		int a;
		void* aPtr = (void*)&a;
		DOCTEST_CHECK(!allocator.Deallocate(aPtr));

		void* nPtr = allocator.Allocate(8);
		DOCTEST_CHECK(!allocator.Deallocate(nPtr));
	}

	DOCTEST_SUBCASE("Owns")
	{
		NullAllocator allocator;

		void* iPtr = nullptr;
		DOCTEST_CHECK(!allocator.Owns(iPtr));

		int a;
		void* aPtr = (void*)&a;
		DOCTEST_CHECK(!allocator.Owns(aPtr));

		void* nPtr = allocator.Allocate(8);
		DOCTEST_CHECK(!allocator.Owns(nPtr));
	}
}