#include "GUI.h"

static Console console = Console();

void OutputCommandList()
{
	Console::WriteLine("�����б�");
	Console::WriteLine("select [fitmode]          fitMode��Ϊ��firstfit nextfit bestfit worstfit");
	Console::WriteLine("alloc [numBytes]");
	Console::WriteLine("dealloc [blockID]");
	Console::WriteLine("show");
	Console::WriteLine("help");
	Console::WriteLine("exit");
}

std::unique_ptr<XAllocator> InitAllocator()
{

	uint32_t numBytes, numBlocks;
	char str[20];
	Console::WriteLine("���������ڴ��С(1-10000000Byte)��");
	Console::ReadFormat("%u", &numBytes);
	assert(numBytes > 0 && numBytes <= 10000000);
	Console::WriteLine("������С����ķֿ���(1-�ڴ��С)��");
	Console::ReadFormat("%u", &numBlocks);
	while (Console::ReadChar() != '\n')
		continue;
	assert(numBlocks > 0 && numBlocks <= numBytes);
	Console::WriteLine("���������㷨(firstfit nextfit bestfit worstfit)��");
	Console::ReadLine(str, 20);
	if (!strcmp(str, "firstfit"))
	{
		return std::make_unique<XAllocator>(numBytes, numBlocks, XAllocator::FitMode::FirstFit);
	}
	else if (!strcmp(str, "nextfit"))
	{
		return std::make_unique<XAllocator>(numBytes, numBlocks, XAllocator::FitMode::NextFit);
	}
	else if (!strcmp(str, "bestfit"))
	{
		return std::make_unique<XAllocator>(numBytes, numBlocks, XAllocator::FitMode::BestFit);
	}
	else if (!strcmp(str, "worstfit"))
	{
		return std::make_unique<XAllocator>(numBytes, numBlocks, XAllocator::FitMode::WorstFit);
	}
	return nullptr;
}

void ShowBlockList(const XAllocator & alloc)
{
	auto blockList = alloc.GetMemoryBlocks();
	Console::WriteLine("�ڴ��ID  ��ַ                �ֽ���      �Ƿ�ʹ��");
	for (const auto& block : blockList)
	{
		if (!block.mergeable)
			Console::SetTextColor(Console::Red);
		Console::WriteFormat("%-10d0x%-18x%-12d%-8s\n", block.id, block.memory, block.size, block.isUsed ? "��" : "  ��");
		Console::SetTextColor();
	}
}
