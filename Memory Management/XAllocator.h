#pragma once
#include <memory>
#include <list>
#include <cassert>
struct MemoryDeleter
{
	void operator()(void * _memory)
	{
		operator delete(_memory);
	}
};

struct MemoryBlock
{
	uint32_t id;	// 编号
	uint32_t size;	// 字节数
	void * memory;	// 内存地址
	bool isUsed;	// 是否被使用
	bool mergeable;	// 是否可以被合并

	bool operator==(const MemoryBlock& rhs)
	{
		return !memcmp(this, &rhs, sizeof(MemoryBlock));
	}
};


class XAllocator
{
public:
	enum class FitMode {
		FirstFit,	// 首次适应
		NextFit,	// 循环首次适应
		BestFit,	// 最佳适应
		WorstFit	// 最坏适应
	};

	// 默认分配1K内存，包含4块内存碎片
	XAllocator(size_t numBytes = 1024, size_t numBlocks = 4, FitMode mode = FitMode::FirstFit);

	// 分配内存
	void * Allocate(size_t numBytes);

	// 回收内存
	void DeAllocate(void * pData);

	// 设置适应模式
	void SetFitMode(FitMode fitMode);

	const std::list<MemoryBlock> GetMemoryBlocks() const;
private:
	// 首次适应
	void * FirstFit(size_t numBytes);
	// 循环首次适应
	void * NextFit(size_t numBytes);
	// 最佳适应
	void * BestFit(size_t numBytes);
	// 最坏适应
	void * WorstFit(size_t numBytes);
	// 拆分成两个内存块，前者是要占用的，后者是剩余未分配的
	void * Split(std::list<MemoryBlock>::iterator it, size_t numBytes);

private:
	std::unique_ptr<void, MemoryDeleter> m_Memory;		// 内存掌管
	std::list<MemoryBlock> m_MemoryBlocks;				// 内存块链表
	std::list<MemoryBlock>::iterator m_CurrBlockIter;	// 当前内存块链表迭代器
	uint32_t m_NextAllocID;								// 下一分配ID
	FitMode m_FitMode;									// 适应模式

};