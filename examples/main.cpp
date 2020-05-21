#include "../src/Dyma.hpp"

#include <cassert> // assert

// Some "extensions" examples to see what kind of allocators be made with the library
#include "DebugAllocator.hpp"
#include "DoubleBufferedAllocator.hpp"

using namespace dyma;

void DebugAllocator_Example();
void DoubleBufferedAllocator_Example();

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
	void* nullPtr = nullAllocator.Allocate(10);
	assert(nullPtr == nullptr);

	// ForbiddenAllocator : Assert on allocation/deallocation
	ForbiddenAllocator forbiddenAllocator;
	//void* forbiddenPtr = forbiddenAllocator.Allocate(10);

	// Mallocator : malloc/free
	Mallocator mallocator;
	void* mallocatorPtr = mallocator.Allocate(10);
	assert(mallocatorPtr != nullptr);
	mallocator.Deallocate(mallocatorPtr);
	assert(mallocatorPtr == nullptr);

	// StackAllocator
	StackAllocator stackAllocator(hMem);
	void* stackPtr = stackAllocator.Allocate(sizeof(unsigned int));

	// You can know if a ptr belongs to an allocator with Owns
	assert(!nullAllocator.Owns(stackPtr));
	assert(stackAllocator.Owns(stackPtr));

	// Memory block are not smart, you should deallocate them by hand
	// Deallocate return true/false depending on the block has been deallocated or not
	// If it does, the block is cleaned to nullptr and size of 0
	assert(stackPtr != nullptr);
	assert(stackAllocator.Deallocate(stackPtr));
	assert(stackPtr == nullptr);

	// PoolAllocator : Can only allocate ptr of the given size
	PoolAllocator poolAllocator(saMem, 16);
	assert(poolAllocator.GetBlockCount() == 64);
	void* poolValidBlock = poolAllocator.Allocate(16);
	assert(poolValidBlock != nullptr);
	void* poolInvalidBlock = poolAllocator.Allocate(32);
	assert(poolInvalidBlock == nullptr);

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
	void* memoryPtr = allocator->Allocate(16);
	assert(memoryPtr != nullptr);
	assert(allocator->Owns(memoryPtr));
	assert(allocator->Deallocate(memoryPtr));
	assert(!allocator->Owns(memoryPtr));
	assert(memoryPtr == nullptr);
	

	DebugAllocator_Example();

	DoubleBufferedAllocator_Example();



	// Don't forget any deallocation :)
	poolAllocator.Deallocate(poolValidBlock);

	return 0;
}

void DebugAllocator_Example()
{
	// Init
	bool debug = true;
	dyma::StackMemory<1024> stackMemory;
	dyma::StackAllocator stackAllocator(stackMemory);
	DebugAllocator debugAllocator(stackAllocator);

	// Use the allocator without knowing the magic behind
	dyma::Allocator* allocator = (debug) ? (dyma::Allocator*)&debugAllocator : (dyma::Allocator*)&stackAllocator;
	void* ptr8 = allocator->Allocate(8);
	void* ptr16 = allocator->Allocate(16);
	void* ptr32 = allocator->Allocate(32);
	allocator->Deallocate(ptr32);

	// Check the stats and find currently used blocks (and leaking blocks)
	assert(debugAllocator.GetUsedSize() == 24);
	assert(debugAllocator.GetPeakSize() == 56);
	for (const DebugAllocator::DebugMemoryBlock& block : debugAllocator.GetBlocks())
	{
		assert((block.ptr == ptr8 && block.size == 8) || (block.ptr == ptr16 && block.size == 16));
	}
}


void DoubleBufferedAllocator_Example()
{
	// Init
	DoubleBufferedAllocator allocator(512);

	for (std::size_t i = 0; i < 123; ++i)
	{
		// Allocate memory from the current buffer
		void* ptr8 = allocator.Allocate(8);
		void* ptr16 = allocator.Allocate(16);

		// ..
		// Simulation using current and/or previous buffer
		// ..

		// Swap the buffer and clear the new one
		allocator.SwapBuffers();
	}
}