#pragma once

#include "../src/Dyma.hpp"

class DoubleBufferedAllocator : public dyma::Allocator
{
public:
	DoubleBufferedAllocator(std::size_t sizePerFrame)
		: mMemory(sizePerFrame * 2)
		, mMemoryViewA(mMemory.GetPointer(), sizePerFrame)
		, mMemoryViewB((void*)(reinterpret_cast<std::uintptr_t>(mMemory.GetPointer()) + sizePerFrame), sizePerFrame)
		, mAllocatorA(mMemoryViewA)
		, mAllocatorB(mMemoryViewB)
		, mCurrentBuffer(&mAllocatorA)
	{
	}

	void* Allocate(std::size_t size) override
	{
		return mCurrentBuffer->Allocate(size);
	}

	bool Deallocate(void*& ptr) override
	{
		return mCurrentBuffer->Deallocate(ptr);
	}

	bool Owns(const void* ptr) const override 
	{
		return mCurrentBuffer->Owns(ptr);
	}

	void SwapBuffers()
	{
		if (mCurrentBuffer == &mAllocatorA)
		{
			mCurrentBuffer = &mAllocatorB;
		}
		else
		{
			mCurrentBuffer = &mAllocatorA;
		}
	}

private:
	dyma::HeapMemory mMemory;
	dyma::MemoryView mMemoryViewA;
	dyma::MemoryView mMemoryViewB;
	dyma::StackAllocator mAllocatorA;
	dyma::StackAllocator mAllocatorB;
	dyma::StackAllocator* mCurrentBuffer;
};