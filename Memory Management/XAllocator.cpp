#include "XAllocator.h"

XAllocator::XAllocator(size_t numBytes, size_t numBlocks, FitMode mode)
	: m_Memory(operator new(numBytes)), m_NextAllocID(0), m_FitMode(mode)
{
	assert(numBlocks > 0);

	// 初始化内存块链表
	size_t stride = numBytes / numBlocks;
	MemoryBlock block;
	block.isUsed = false;
	block.mergeable = false;
	block.size = stride;
	for (size_t i = 0; i < numBlocks; ++i)
	{
		block.id = m_NextAllocID++;
		block.memory = reinterpret_cast<char *>(m_Memory.get()) + i * stride;
		if (i == numBlocks - 1)
			block.size = stride + numBytes % numBlocks;
		m_MemoryBlocks.push_back(block);
	}

	// 初始化迭代器
	m_CurrBlockIter = m_MemoryBlocks.begin();
}

void * XAllocator::Allocate(size_t numBytes)
{
	switch (m_FitMode)
	{
	case FitMode::FirstFit: return FirstFit(numBytes);
	case FitMode::NextFit: return NextFit(numBytes);
	case FitMode::BestFit: return BestFit(numBytes);
	case FitMode::WorstFit: return WorstFit(numBytes);
	}
	return nullptr;
}

void XAllocator::DeAllocate(void * pData)
{
	auto it = m_MemoryBlocks.begin();
	while (it != m_MemoryBlocks.end())
	{
		if (it->memory == pData && it->isUsed)
			break;
		++it;
	}

	if (it == m_MemoryBlocks.end())
	{
		throw std::exception("wrong pointer!");
	}

	it->isUsed = false;


	auto rightIt = it;
	++rightIt;
	// 本身不可被合并，只能考虑右边的
	if (!it->mergeable)
	{
		if (rightIt != m_MemoryBlocks.end() && !rightIt->isUsed)
		{
			it->size += rightIt->size;
			m_MemoryBlocks.erase(rightIt);
		}
	}
	else
	{
		auto leftIt = it;
		--leftIt;
		// 不能和右边合并的情况
		if (rightIt == m_MemoryBlocks.end() || rightIt->isUsed || !rightIt->mergeable)
		{
			// 只能和左边合并的情况
			if (!leftIt->isUsed)
			{
				leftIt->size += it->size;
				m_MemoryBlocks.erase(it);
			}
			// 两边都不能合并的情况
		}
		// 只能和右边合并的情况
		else if (leftIt->isUsed && !rightIt->isUsed)
		{
			rightIt->size += it->size;
			m_MemoryBlocks.erase(it);
		}
		// 两边一起合并的情况
		else
		{
			leftIt->isUsed = false;
			leftIt->size += it->size + rightIt->size;
			auto edIt = rightIt;
			edIt++;
			m_MemoryBlocks.erase(it, edIt);
		}
	}
}

void XAllocator::SetFitMode(FitMode fitMode)
{
	m_FitMode = fitMode;
}

const std::list<MemoryBlock> XAllocator::GetMemoryBlocks() const
{
	return m_MemoryBlocks;
}

void * XAllocator::FirstFit(size_t numBytes)
{
	auto it = m_MemoryBlocks.begin();
	while (it != m_MemoryBlocks.end())
	{
		if (!it->isUsed)
		{
			// 大小匹配则直接分配该内存
			if (it->size == numBytes)
			{
				it->isUsed = true;
				return it->memory;
			}
			// 大小不匹配则拆成两块内存
			else if (it->size > numBytes)
			{
				return Split(it, numBytes);
			}
		}
		++it;
	}

	return nullptr;
}

void * XAllocator::NextFit(size_t numBytes)
{
	size_t len = m_MemoryBlocks.size();
	for (size_t i = 0; i < len; ++i, ++m_CurrBlockIter)
	{
		// 到达尾部要回到开头
		if (m_CurrBlockIter == m_MemoryBlocks.end())
			m_CurrBlockIter = m_MemoryBlocks.begin();
		if (!m_CurrBlockIter->isUsed)
		{
			// 大小匹配则直接分配该内存
			if (m_CurrBlockIter->size == numBytes)
			{
				m_CurrBlockIter->isUsed = true;
				++m_CurrBlockIter;
				return m_CurrBlockIter->memory;
			}
			// 大小不匹配则拆成两块内存
			else if (m_CurrBlockIter->size > numBytes)
			{
				return Split(m_CurrBlockIter, numBytes);
			}
		}
	}

	return nullptr;
}

void * XAllocator::BestFit(size_t numBytes)
{
	auto it = m_MemoryBlocks.begin();
	auto minIt = m_MemoryBlocks.end();
	size_t minBlockBytes = SIZE_MAX;
	while (it != m_MemoryBlocks.end())
	{
		if (!it->isUsed)
		{
			// 大小匹配则直接分配该内存
			if (it->size == numBytes)
			{
				it->isUsed = true;
				return it->memory;
			}
			// 大小不匹配则寻找更小的
			else if (it->size > numBytes && it->size < minBlockBytes)
			{
				minBlockBytes = it->size;
				minIt = it;
			}
		}
		++it;
	}

	if (minIt != m_MemoryBlocks.end())
	{
		return Split(minIt, numBytes);
	}

	return nullptr;
}

void * XAllocator::WorstFit(size_t numBytes)
{
	auto it = m_MemoryBlocks.begin();
	auto maxIt = m_MemoryBlocks.end();
	size_t maxBlockBytes = 0;
	while (it != m_MemoryBlocks.end())
	{
		if (!it->isUsed)
		{
			// 大小匹配则直接分配该内存
			if (it->size == numBytes)
			{
				it->isUsed = true;
				return it->memory;
			}
			// 大小不匹配则寻找更大的
			else if (it->size > numBytes && it->size > maxBlockBytes)
			{
				maxBlockBytes = it->size;
				maxIt = it;
			}
		}
		++it;
	}

	if (maxIt != m_MemoryBlocks.end())
	{
		return Split(maxIt, numBytes);
	}

	return nullptr;
}

void * XAllocator::Split(std::list<MemoryBlock>::iterator it, size_t numBytes)
{
	//
	// 新内存块 -> 当前内存块
	// 占用        未占用

	// 新的内存块为剩余内存块
	MemoryBlock block;
	block.id = m_NextAllocID++;
	block.isUsed = true;
	block.mergeable = it->mergeable;
	block.memory = it->memory;
	block.size = numBytes;
	// 修改当前内存块
	it->mergeable = true;
	it->memory = reinterpret_cast<char *>(it->memory) + numBytes;
	it->size -= numBytes;
	// 插入到it前面
	m_MemoryBlocks.insert(it, block);
	return block.memory;
}
