#include "../src/Dyma.hpp"
#include "doctest.h"

using namespace dyma;

DOCTEST_TEST_CASE("NullAllocator")
{
	DOCTEST_SUBCASE("Allocate")
	{
		NullAllocator allocator;
		DOCTEST_CHECK(!allocator.Allocate(0).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(1).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(2).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(3).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(4).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(8).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(16).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(32).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(64).IsValid());
		DOCTEST_CHECK(!allocator.Allocate(128).IsValid());
	}

	DOCTEST_SUBCASE("Deallocate")
	{
		NullAllocator allocator;
		int a;
		MemoryBlock invalidBlock;
		MemoryBlock randomBlock(&a, sizeof(a));
		MemoryBlock block = allocator.Allocate(8);
		DOCTEST_CHECK(!block.IsValid());
		DOCTEST_CHECK(!allocator.Deallocate(invalidBlock));
		DOCTEST_CHECK(!allocator.Deallocate(randomBlock));
		DOCTEST_CHECK(!allocator.Deallocate(block));
	}

	DOCTEST_SUBCASE("Owns")
	{
		NullAllocator allocator;
		int a;
		MemoryBlock invalidBlock;
		MemoryBlock randomBlock(&a, sizeof(a));
		MemoryBlock block = allocator.Allocate(8);
		DOCTEST_CHECK(!block.IsValid());
		DOCTEST_CHECK(!allocator.Owns(invalidBlock));
		DOCTEST_CHECK(!allocator.Owns(randomBlock));
		DOCTEST_CHECK(!allocator.Owns(block));
	}
}