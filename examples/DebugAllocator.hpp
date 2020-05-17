#pragma once

#include "../src/Dyma.hpp"

#include <vector>

class DebugAllocator : public dyma::Allocator
{
public:
	DebugAllocator(dyma::Allocator& allocatorToDebug)
		: mAllocator(allocatorToDebug)
		, mAllocationCount(0)
		, mDeallocationCount(0)
		, mUsedSize(0)
		, mPeakSize(0)
	{
	}

	dyma::MemoryBlock Allocate(std::size_t size) override
	{
		dyma::MemoryBlock block = mAllocator.Allocate(size);
		if (block.IsValid())
		{
			mAllocationCount++;
			mUsedSize++;
			if (mUsedSize > mPeakSize)
			{
				mPeakSize = mUsedSize;
			}
			mBlockSizes.push_back(size);
		}
		return block;
	}

	bool Deallocate(dyma::MemoryBlock& block) override
	{
		const std::size_t blockSize = block.size;
		const bool result = mAllocator.Deallocate(block);
		if (result)
		{
			mDeallocationCount++;
			mUsedSize -= blockSize;
		}
		return result;
	}

	bool Owns(const dyma::MemoryBlock& block) const override 
	{
		return mAllocator.Owns(block);
	}

	std::size_t GetAllocationCount() const { return mAllocationCount; }
	std::size_t GetDeallocationCount() const { return mDeallocationCount; }
	std::size_t GetUsedSize() const { return mUsedSize; }
	std::size_t GetPeakSize() const { return mPeakSize; }
	const std::vector<std::size_t> GetBlockSizes() const { return mBlockSizes; }

private:
	dyma::Allocator& mAllocator;

	std::size_t mAllocationCount;
	std::size_t mDeallocationCount;

	std::size_t mUsedSize;
	std::size_t mPeakSize;

	std::vector<std::size_t> mBlockSizes;
};