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

	dyma::MemoryBlock Allocate(std::size_t size) override
	{
		return mCurrentBuffer->Allocate(size);
	}

	bool Deallocate(dyma::MemoryBlock& block) override
	{
		return mCurrentBuffer->Deallocate(block);
	}

	bool Owns(const dyma::MemoryBlock& block) const override 
	{
		return mCurrentBuffer->Owns(block);
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