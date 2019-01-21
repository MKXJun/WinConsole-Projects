#include "GUI.h"
#include <sstream>
#include <map>

int main()
{
	Console::SetBufferSize(80, 40);
	Console::SetWindowSize(80, 40);
	Console::SetTitle("存储管理");
	std::unique_ptr<XAllocator> pAlloc = InitAllocator();
	std::map<uint32_t, void*> memoryMap;
	std::istringstream iss;
	char str[200];
	uint32_t num1, currAlloc = pAlloc->GetMemoryBlocks().size();
	system("cls");
	OutputCommandList();
	while (true)
	{
		Console::Write(">>");
		Console::ReadLine(str, 200);
		
		iss.rdbuf()->str(str);
		iss.clear();
		iss.seekg(0);
		iss >> str;
		if (!strcmp(str, "select"))
		{
			system("cls");
			iss >> str;
			if (!strcmp(str, "firstfit"))
				pAlloc->SetFitMode(XAllocator::FitMode::FirstFit);
			else if (!strcmp(str, "nextfit"))
				pAlloc->SetFitMode(XAllocator::FitMode::NextFit);
			else if (!strcmp(str, "bestfit"))
				pAlloc->SetFitMode(XAllocator::FitMode::BestFit);
			else if (!strcmp(str, "worstfit"))
				pAlloc->SetFitMode(XAllocator::FitMode::WorstFit);
			else
				Console::WriteLine("设置失败！");
		}
		else if (!strcmp(str, "alloc"))
		{
			system("cls");
			if (iss >> num1)
			{
				void * ptr = pAlloc->Allocate(num1);
				if (ptr != nullptr)
				{
					memoryMap[currAlloc++] = ptr;
					ShowBlockList(*pAlloc);
				}
				else
				{
					Console::WriteLine("分配失败！");
				}
			}
			else
			{
				Console::WriteLine("语句不合法！");
			}
		}
		else if (!strcmp(str, "dealloc"))
		{
			if (iss >> num1)
			{
				auto it = memoryMap.find(num1);
				if (it != memoryMap.end())
				{
					pAlloc->DeAllocate(it->second);
					memoryMap.erase(num1);
					system("cls");
					ShowBlockList(*pAlloc);
				}
				else
				{
					system("cls");
					Console::WriteLine("删除失败！");
				}
					
			}
			else
			{
				Console::WriteLine("语句不合法！");
			}
		}
		else if (!strcmp(str, "show"))
		{
			system("cls");
			ShowBlockList(*pAlloc);
		}
		else if (!strcmp(str, "exit"))
			break;
		else if (!strcmp(str, "help"))
		{
			system("cls");
			OutputCommandList();
		}
		else
		{
			system("cls");
			Console::WriteLine("语句不合法！");
		}
	}
	return 0;
}