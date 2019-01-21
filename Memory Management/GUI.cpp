#include "GUI.h"

static Console console = Console();

void OutputCommandList()
{
	Console::WriteLine("命令列表：");
	Console::WriteLine("select [fitmode]          fitMode可为：firstfit nextfit bestfit worstfit");
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
	Console::WriteLine("输入分配的内存大小(1-10000000Byte)：");
	Console::ReadFormat("%u", &numBytes);
	assert(numBytes > 0 && numBytes <= 10000000);
	Console::WriteLine("输入最小允许的分块数(1-内存大小)：");
	Console::ReadFormat("%u", &numBlocks);
	while (Console::ReadChar() != '\n')
		continue;
	assert(numBlocks > 0 && numBlocks <= numBytes);
	Console::WriteLine("输入适配算法(firstfit nextfit bestfit worstfit)：");
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
	Console::WriteLine("内存块ID  地址                字节数      是否使用");
	for (const auto& block : blockList)
	{
		if (!block.mergeable)
			Console::SetTextColor(Console::Red);
		Console::WriteFormat("%-10d0x%-18x%-12d%-8s\n", block.id, block.memory, block.size, block.isUsed ? "是" : "  否");
		Console::SetTextColor();
	}
}
