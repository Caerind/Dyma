#include "Dyma.hpp"

#include <cstdlib> // malloc/calloc/realloc/free
#include <cassert> // assert

namespace dyma
{

void* Malloc(std::size_t size)
{
	return (size > 0) ? std::malloc(size) : nullptr;
}

void* Calloc(std::size_t num, std::size_t size)
{
	return (num > 0 && size > 0) ? std::calloc(num, size) : nullptr;
}

void* Realloc(void* ptr, std::size_t newSize)
{
	return (newSize > 0) ? std::realloc(ptr, newSize) : nullptr;
}

void Free(void* ptr)
{
	if (ptr != nullptr)
	{
		std::free(ptr);
	}
}

void* AlignedMalloc(std::size_t size, std::size_t alignment)
{
	assert(alignment >= 1);
	assert(alignment <= 128);
	assert((alignment & (alignment - 1)) == 0);
	const std::size_t requestSize = size + alignment;
	const void* rawMemory = Malloc(requestSize);
	if (rawMemory == nullptr)
	{
		return nullptr;
	}
	const std::uintptr_t rawAddress = reinterpret_cast<std::uintptr_t>(rawMemory);
	const std::uintptr_t misalignment = (rawAddress & (alignment - 1));
	const std::ptrdiff_t adjustment = alignment - misalignment;
	const std::uintptr_t alignedAddress = rawAddress + adjustment;
	assert(adjustment < 256);
	std::uint8_t* pAlignedMemory = reinterpret_cast<std::uint8_t*>(alignedAddress);
	pAlignedMemory[-1] = static_cast<std::uint8_t>(adjustment);
	return static_cast<void*>(pAlignedMemory);
}

void AlignedFree(void* ptr)
{
	if (ptr != nullptr)
	{
		const std::uint8_t* pAlignedMemory = reinterpret_cast<std::uint8_t*>(ptr);
		const std::uintptr_t alignedAddress = reinterpret_cast<std::uintptr_t>(ptr);
		const std::ptrdiff_t adjustment = static_cast<std::ptrdiff_t>(pAlignedMemory[-1]);
		const std::uintptr_t rawAddress = alignedAddress - adjustment;
		Free(reinterpret_cast<void*>(rawAddress));
	}
}

std::size_t RoundToAlignment(std::size_t size, std::size_t alignment)
{
	return (size + (alignment - 1)) & -alignment;
}

MemoryBlock::MemoryBlock()
	: ptr(nullptr)
	, size(0)
{
}

MemoryBlock::MemoryBlock(void* _ptr, std::size_t _size)
	: ptr(_ptr)
	, size(_size)
{
}

bool MemoryBlock::IsValid() const
{
	return ptr != nullptr && size > 0;
}

void MemoryBlock::Reset()
{
	ptr = nullptr;
	size = 0;
}

void* MemoryBlock::GetEndPointer()
{
	return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(ptr) + size);
}

const void* MemoryBlock::GetEndPointer() const
{
	return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(ptr) + size);
}

bool MemoryBlock::operator==(const MemoryBlock& other) const
{
	return ptr == other.ptr && size == other.size;
}

bool MemoryBlock::operator!=(const MemoryBlock& other) const
{
	return !operator==(other);
}

const void* MemorySource::GetEndPointer() const
{
	return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(GetPointer()) + GetSize());
}

bool MemorySource::Owns(const void* ptr) const
{
	return GetPointer() <= ptr && ptr < GetEndPointer();
}

bool MemorySource::Owns(const MemoryBlock& block) const
{
	return GetPointer() <= block.ptr && block.GetEndPointer() <= GetEndPointer();
}

MemoryBlock MemorySource::GetMemoryBlock() const
{
	return MemoryBlock((void*)GetPointer(), GetSize());
}

bool MemorySource::OwnsMemory() const
{
	return true;
}

const void* NullMemory::GetPointer() const 
{
	return nullptr;
}

std::size_t NullMemory::GetSize() const
{
	return 0;
}

std::size_t NullMemory::GetAlignment() const
{
	return 0;
}

bool NullMemory::OwnsMemory() const
{
	return false;
}

HeapMemory::HeapMemory(std::size_t bytes, std::size_t alignment /*= 0*/)
	: mMemory((alignment == 0) ? Malloc(bytes) : AlignedMalloc(bytes, alignment))
	, mSize((mMemory != nullptr) ? bytes : 0)
	, mAlignment((alignment == 0) ? 16 : alignment)
	, mAlignedByUser(alignment != 0)
{
}

HeapMemory::~HeapMemory()
{
	if (!mAlignedByUser)
	{
		Free(mMemory);
	}
	else
	{
		AlignedFree(mMemory);
	}
}

const void* HeapMemory::GetPointer() const
{
	return mMemory;
}

std::size_t HeapMemory::GetSize() const
{
	return mSize;
}

std::size_t HeapMemory::GetAlignment() const
{
	return mAlignment;
}

MemoryView::MemoryView(const void* begin, std::size_t bytes, std::size_t alignment /*= alignof(std::max_align_t)*/)
	: mBegin(begin)
	, mSize(bytes)
	, mAlignment(alignment)
{
}

const void* MemoryView::GetPointer() const
{
	return mBegin;
}

std::size_t MemoryView::GetSize() const
{
	return mSize;
}

std::size_t MemoryView::GetAlignment() const
{
	return mAlignment;
}

bool MemoryView::OwnsMemory() const
{
	return false;
}

MemoryBlock NullAllocator::Allocate(std::size_t size)
{
	return MemoryBlock();
}

bool NullAllocator::Deallocate(MemoryBlock& block)
{
	return false;
}

bool NullAllocator::Owns(const MemoryBlock& block) const
{
	return false;
}

MemoryBlock ForbiddenAllocator::Allocate(std::size_t size)
{
	assert(false);
	return MemoryBlock();
}

bool ForbiddenAllocator::Deallocate(MemoryBlock& block)
{
	assert(false);
	return false;
}

bool ForbiddenAllocator::Owns(const MemoryBlock& block) const
{
	return false;
}

MemoryBlock Mallocator::Allocate(std::size_t size)
{
	MemoryBlock block;
	if (size > 0)
	{
		block.ptr = Malloc(size);
		if (block.ptr != nullptr)
		{
			block.size = size;
		}
	}
	return block;
}

bool Mallocator::Deallocate(MemoryBlock& block)
{
	if (block.ptr != nullptr)
	{
		Free(block.ptr);
		block.ptr = nullptr;
		block.size = 0;
		return true;
	}
	return false;
}

bool Mallocator::Owns(const MemoryBlock& block) const
{
	return false;
}

StackAllocator::StackAllocator(MemorySource& source)
	: mSource(source)
	, mPointer(reinterpret_cast<std::uintptr_t>(mSource.GetPointer()))
{
}

MemoryBlock StackAllocator::Allocate(std::size_t size)
{
	MemoryBlock	block;
	const std::size_t alignedSize = RoundToAlignment(size, GetAlignment());
	if (size > 0 && alignedSize <= GetRemainingSize())
	{
		block.ptr = reinterpret_cast<void*>(mPointer);
		if (block.ptr != nullptr)
		{
			block.size = size;
		}
		mPointer += alignedSize;
	}
	return block;
}

bool StackAllocator::Deallocate(MemoryBlock& block)
{
	// Only the last allocated block can be released
	const std::uintptr_t blockBytePtr = reinterpret_cast<std::uintptr_t>(block.ptr);
	if (block.IsValid() && blockBytePtr + RoundToAlignment(block.size, GetAlignment()) == mPointer)
	{
		mPointer = blockBytePtr;
		block.Reset();
		return true;
	}
	return false;
}

bool StackAllocator::Owns(const MemoryBlock& block) const
{
	return mSource.Owns(block) && reinterpret_cast<std::uintptr_t>(block.ptr) < mPointer;
}

MemoryBlock StackAllocator::AllocateAll()
{
	MemoryBlock block;
	const std::size_t remainingSize = GetRemainingSize();
	if (remainingSize > 0)
	{
		block.ptr = reinterpret_cast<void*>(mPointer);
		block.size = remainingSize;
		mPointer += remainingSize;
	}
	return block;
}

void StackAllocator::DeallocateAll()
{
	mPointer = reinterpret_cast<std::uintptr_t>(mSource.GetPointer());
}

std::size_t StackAllocator::GetUsedSize() const
{
	return mPointer - reinterpret_cast<std::uintptr_t>(mSource.GetPointer());
}

std::size_t StackAllocator::GetRemainingSize() const
{
	return (reinterpret_cast<std::uintptr_t>(mSource.GetPointer()) + mSource.GetSize()) - mPointer;
}

std::size_t StackAllocator::GetSize() const
{
	return mSource.GetSize();
}

std::size_t StackAllocator::GetAlignment() const
{
	return mSource.GetAlignment();
}

PoolAllocator::PoolAllocator(MemorySource& source, std::size_t blockSize)
	: mSource(source)
	, mRootNode((Node*)mSource.GetPointer())
	, mBlockSize(blockSize)
{
	assert(mBlockSize > 0);
	assert(mBlockSize >= sizeof(void*));
	assert(mSource.GetSize() % mBlockSize == 0);

	Node* nodePtr = mRootNode;
	for (std::size_t i = 1; i < GetBlockCount() - 1; ++i)
	{
		Node* node = nodePtr;
		node->next = (Node*)(reinterpret_cast<std::uintptr_t>(mSource.GetPointer()) + i * mBlockSize);
		nodePtr = node->next;
	}
	nodePtr->next = nullptr;
}

MemoryBlock PoolAllocator::Allocate(std::size_t size)
{
	MemoryBlock block;
	if (size == mBlockSize && mRootNode != nullptr)
	{
		block.ptr = (void*)mRootNode;
		block.size = mBlockSize;
		mRootNode = mRootNode->next;
	}
	return block;
}

bool PoolAllocator::Deallocate(MemoryBlock& block)
{
	if (block.size == mBlockSize)
	{
		Node* node = (Node*)block.ptr;
		node->next = mRootNode;
		mRootNode = node;
		block.ptr = nullptr;
		block.size = 0;
		return true;
	}
	return false;
}

bool PoolAllocator::Owns(const MemoryBlock& block) const
{
	return block.size == mBlockSize && mSource.Owns(block);
}

std::size_t PoolAllocator::GetBlockSize() const
{
	return mBlockSize;
}

std::size_t PoolAllocator::GetBlockCount() const
{
	return mSource.GetSize() / mBlockSize;
}

std::size_t PoolAllocator::GetSize() const
{
	return mSource.GetSize();
}

FallbackAllocator::FallbackAllocator(Allocator& primaryAllocator, Allocator& secondaryAllocator)
	: mPrimary(primaryAllocator)
	, mSecondary(secondaryAllocator)
{
}

MemoryBlock FallbackAllocator::Allocate(std::size_t size)
{
	MemoryBlock	block = mPrimary.Allocate(size);
	if (!block.IsValid())
	{
		block = mSecondary.Allocate(size);
	}
	return block;
}

bool FallbackAllocator::Deallocate(MemoryBlock& block)
{
	if (mPrimary.Owns(block))
	{
		return mPrimary.Deallocate(block);
	}
	else
	{
		return mSecondary.Deallocate(block);
	}
}

bool FallbackAllocator::Owns(const MemoryBlock& block) const
{
	return mPrimary.Owns(block) || mSecondary.Owns(block);
}

SegregatorAllocator::SegregatorAllocator(std::size_t threshold, Allocator& smaller, Allocator& larger)
	: mSmallerAllocator(smaller)
	, mLargerAllocator(larger)
	, mThreshold(threshold)
{
}

MemoryBlock SegregatorAllocator::Allocate(std::size_t size)
{
	if (size <= mThreshold)
	{
		return mSmallerAllocator.Allocate(size);
	}
	else
	{
		return mLargerAllocator.Allocate(size);
	}
}

bool SegregatorAllocator::Deallocate(MemoryBlock& block)
{
	if (block.size <= mThreshold)
	{
		return mSmallerAllocator.Deallocate(block);
	}
	else
	{
		return mLargerAllocator.Deallocate(block);
	}
}

bool SegregatorAllocator::Owns(const MemoryBlock& block) const
{
	if (block.size <= mThreshold)
	{
		return mSmallerAllocator.Owns(block);
	}
	else
	{
		return mLargerAllocator.Owns(block);
	}
}

std::size_t SegregatorAllocator::GetThreshold() const
{
	return mThreshold;
}

} // namespace dyma