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
	uint32_t id;	// ���
	uint32_t size;	// �ֽ���
	void * memory;	// �ڴ��ַ
	bool isUsed;	// �Ƿ�ʹ��
	bool mergeable;	// �Ƿ���Ա��ϲ�

	bool operator==(const MemoryBlock& rhs)
	{
		return !memcmp(this, &rhs, sizeof(MemoryBlock));
	}
};


class XAllocator
{
public:
	enum class FitMode {
		FirstFit,	// �״���Ӧ
		NextFit,	// ѭ���״���Ӧ
		BestFit,	// �����Ӧ
		WorstFit	// ���Ӧ
	};

	// Ĭ�Ϸ���1K�ڴ棬����4���ڴ���Ƭ
	XAllocator(size_t numBytes = 1024, size_t numBlocks = 4, FitMode mode = FitMode::FirstFit);

	// �����ڴ�
	void * Allocate(size_t numBytes);

	// �����ڴ�
	void DeAllocate(void * pData);

	// ������Ӧģʽ
	void SetFitMode(FitMode fitMode);

	const std::list<MemoryBlock> GetMemoryBlocks() const;
private:
	// �״���Ӧ
	void * FirstFit(size_t numBytes);
	// ѭ���״���Ӧ
	void * NextFit(size_t numBytes);
	// �����Ӧ
	void * BestFit(size_t numBytes);
	// ���Ӧ
	void * WorstFit(size_t numBytes);
	// ��ֳ������ڴ�飬ǰ����Ҫռ�õģ�������ʣ��δ�����
	void * Split(std::list<MemoryBlock>::iterator it, size_t numBytes);

private:
	std::unique_ptr<void, MemoryDeleter> m_Memory;		// �ڴ��ƹ�
	std::list<MemoryBlock> m_MemoryBlocks;				// �ڴ������
	std::list<MemoryBlock>::iterator m_CurrBlockIter;	// ��ǰ�ڴ�����������
	uint32_t m_NextAllocID;								// ��һ����ID
	FitMode m_FitMode;									// ��Ӧģʽ

};