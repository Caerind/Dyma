#pragma once

#include "../src/Dyma.hpp"

#include <vector>

class DebugAllocator : public dyma::Allocator
{
public:
	struct DebugMemoryBlock
	{
		void* ptr;
		std::size_t size;
	};

	DebugAllocator(dyma::Allocator& allocatorToDebug)
		: mAllocator(allocatorToDebug)
		, mAllocationCount(0)
		, mDeallocationCount(0)
		, mUsedSize(0)
		, mPeakSize(0)
	{
	}

	void* Allocate(std::size_t size) override
	{
		void* ptr = mAllocator.Allocate(size);
		if (ptr != nullptr)
		{
			mAllocationCount++;

			DebugMemoryBlock block;
			block.ptr = ptr;
			block.size = size;
			mBlocks.push_back(block);

			mUsedSize += size;
			if (mUsedSize > mPeakSize)
			{
				mPeakSize = mUsedSize;
			}
		}
		return ptr;
	}

	bool Deallocate(void*& ptr) override
	{
		const void* ptrBeforeDealloc = ptr;
		const bool result = mAllocator.Deallocate(ptr);
		if (result)
		{
			mDeallocationCount++;

			std::size_t deallocationSize = 0;
			const std::size_t blockCount = mBlocks.size();
			for (std::size_t i = 0; i < blockCount; ++i)
			{
				if (ptrBeforeDealloc == mBlocks[i].ptr)
				{
					deallocationSize = mBlocks[i].size;
					mBlocks.erase(mBlocks.begin() + i);
				}
			}

			mUsedSize -= deallocationSize;
		}
		return result;
	}

	bool Owns(const void* ptr) const override 
	{
		return mAllocator.Owns(ptr);
	}

	std::size_t GetAllocationCount() const { return mAllocationCount; }
	std::size_t GetDeallocationCount() const { return mDeallocationCount; }
	std::size_t GetUsedSize() const { return mUsedSize; }
	std::size_t GetPeakSize() const { return mPeakSize; }
	const std::vector<DebugMemoryBlock>& GetBlocks() const { return mBlocks; }

private:
	dyma::Allocator& mAllocator;

	std::size_t mAllocationCount;
	std::size_t mDeallocationCount;

	std::size_t mUsedSize;
	std::size_t mPeakSize;

	std::vector<DebugMemoryBlock> mBlocks;
};