#include "../src/Dyma.hpp"

#include <cassert> // assert

using namespace dyma;

int main()
{
	// Memory sources
	NullMemory nMem;
	StackMemory<1024> sMem;
	StackMemory<1024, 16> saMem;
	HeapMemory hMem(1024);
	HeapMemory haMem(1024, 16);

	// Memory views : Can be used to split a memory source into many "sources"
	MemoryView viewA(sMem.GetPointer(), 512);
	MemoryView viewB((void*)(reinterpret_cast<std::uintptr_t>(sMem.GetPointer()) + 512), 512);

	// All the memory sources share the same interface : MemorySource
	MemorySource* memories[] = {
		&sMem,
		&saMem,
		&hMem,
		&haMem
	};
	std::size_t reservedMemory = 0;
	for (std::size_t i = 0; i < 4; ++i)
	{
		reservedMemory += memories[i]->GetSize();
	}
	assert(reservedMemory == 4096);

	// NullAllocator : Does nothing
	NullAllocator nullAllocator;
	MemoryBlock nullBlock = nullAllocator.Allocate(10);
	assert(!nullBlock.IsValid());

	// ForbiddenAllocator : Assert on allocation/deallocation
	ForbiddenAllocator forbiddenAllocator;
	//MemoryBlock forbiddenBlock = forbiddenAllocator.Allocate(10);

	// Mallocator : malloc/free
	Mallocator mallocator;
	MemoryBlock mallocatorBlock = mallocator.Allocate(10);
	assert(mallocatorBlock.IsValid());
	mallocator.Deallocate(mallocatorBlock);
	assert(!mallocatorBlock.IsValid());

	// StackAllocator
	StackAllocator stackAllocator(hMem);
	MemoryBlock stackBlock = stackAllocator.Allocate(sizeof(unsigned int));

	// MemoryBlock : ptr & size of the block
	assert(stackBlock.ptr != nullptr);
	assert(stackBlock.size == sizeof(unsigned int));
	unsigned int* intPtr = reinterpret_cast<unsigned int*>(stackBlock.ptr);
	*intPtr = 12345;

	// You can know if a block belongs to an allocator with Owns
	assert(!nullAllocator.Owns(stackBlock));
	assert(stackAllocator.Owns(stackBlock));

	// Memory block are not smart, you should deallocate them by hand
	// Deallocate return true/false depending on the block has been deallocated or not
	// If it does, the block is cleaned to nullptr and size of 0
	assert(stackBlock.IsValid());
	assert(stackAllocator.Deallocate(stackBlock));
	assert(!stackBlock.IsValid());

	// PoolAllocator : Can only allocate block of the given size
	PoolAllocator poolAllocator(saMem, 16);
	assert(poolAllocator.GetBlockCount() == 64);
	MemoryBlock poolValidBlock = poolAllocator.Allocate(16);
	assert(poolValidBlock.IsValid());
	MemoryBlock poolInvalidBlock = poolAllocator.Allocate(32);
	assert(!poolInvalidBlock.IsValid());

	// FallbackAllocator : Try the primary allocator, then the secondary if the primary failed
	FallbackAllocator fallbackAllocator(stackAllocator, mallocator);
	FallbackAllocator fallbackAllocator2(poolAllocator, forbiddenAllocator);

	// SegregatorAllocator : Depending on the size of the allocation, it will choose the first or the second
	// It can be combined multiple times to create powerful allocators
	SegregatorAllocator smallAllocator(8, mallocator, fallbackAllocator2);
	SegregatorAllocator largeAllocator(64, stackAllocator, forbiddenAllocator);
	SegregatorAllocator segregatorAllocator(16, smallAllocator, largeAllocator);

	// All the allocators share the same interface : Allocator
	Allocator* allocator = &segregatorAllocator;
	MemoryBlock memoryBlock = allocator->Allocate(16);
	assert(memoryBlock.IsValid());
	assert(memoryBlock.size == 16);
	assert(allocator->Owns(memoryBlock));
	assert(allocator->Deallocate(memoryBlock));
	assert(!allocator->Owns(memoryBlock));
	assert(!memoryBlock.IsValid());

	return 0;
}