#include "BTree.h"
#include <iostream>
#include <Windows.h>


BTree<int, int, 3> t3;
BTree<int, int, 4> t4;
BTree<int, int, 5> t5;
BTree<int, int, 6> t6;
int order;
HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

void TreeCreate(std::istringstream& iss);
void TreeInsert(std::istringstream& iss);
void TreeErase(std::istringstream& iss);
void TreeShow(std::istringstream& iss);
void TreeFind(std::istringstream& iss);
void TreeClear();

void ShowHelp();

void Parser(std::string str)
{
	SetConsoleTextAttribute(handle, 0xF0);

	std::istringstream iss(str);
	std::string word;
	iss >> word;
	if (word == "help")
	{
		ShowHelp();
		putchar('>');
		SetConsoleTextAttribute(handle, 0xF4);
		return;
	}

	if (word == "create")
	{
		TreeCreate(iss);
		SetConsoleTextAttribute(handle, 0xF4);
		return;
	}

	if (order < 3 || order > 6)
	{
		puts("未创建B树！请先使用create命令。");
		putchar('>');
		SetConsoleTextAttribute(handle, 0xF4);
		return;
	}

	if (word == "insert")
		TreeInsert(iss);
	else if (word == "erase")
		TreeErase(iss);
	else if (word == "show")
		TreeShow(iss);
	else if (word == "find")
		TreeFind(iss);
	else if (word == "clear")
		TreeClear();
	else if (word == "quit")
		exit(0);
	else
	{
		puts("未知命令！输入help查看帮助。");
	}
	putchar('>');
	SetConsoleTextAttribute(handle, 0xF4);
}

void TreeCreate(std::istringstream& iss)
{
	iss >> order;
	if (order < 3 || order > 6)
	{
		puts("不合法参数！");
		order = 0;
	}
	else
	{
		switch (order)
		{
		case 3: t3.Clear(); break;
		case 4: t4.Clear(); break;
		case 5: t5.Clear(); break;
		case 6: t6.Clear(); break;
		}
		puts("创建成功！");
	}
		
	putchar('>');
}

void TreeInsert(std::istringstream& iss)
{
	std::vector<int> args;
	int num;
	while (iss >> num)
		args.push_back(num);

	int count = 0;
	switch (order)
	{
	case 3: for (int val : args) if (t3.Insert(val, val)) count++; break;
	case 4: for (int val : args) if (t4.Insert(val, val)) count++; break;
	case 5: for (int val : args) if (t5.Insert(val, val)) count++; break;
	case 6: for (int val : args) if (t6.Insert(val, val)) count++; break;
	}
	std::cout << "成功插入" + std::to_string(count) + "个元素！" << std::endl;
}

void TreeErase(std::istringstream& iss)
{
	std::vector<int> args;
	int num;
	while (iss >> num)
		args.push_back(num);
	int count = 0;
	switch (order)
	{
	case 3: for (int val : args) if (t3.Erase(val)) count++; break;
	case 4: for (int val : args) if (t4.Erase(val)) count++; break;
	case 5: for (int val : args) if (t5.Erase(val)) count++; break;
	case 6: for (int val : args) if (t6.Erase(val)) count++; break;
	}
	std::cout << "成功删除" + std::to_string(count) + "个元素！" << std::endl;
}

void TreeShow(std::istringstream& iss)
{
	switch (order)
	{
	case 3: t3.Show(); break;
	case 4: t4.Show(); break;
	case 5: t5.Show(); break;
	case 6: t6.Show(); break;
	}
}

void TreeFind(std::istringstream& iss)
{
	KeyValuePair<int, int> *p = nullptr;
	int num;
	iss >> num;
	if (!iss)
	{
		std::cout << "输入格式有误！" << std::endl;
		return;
	}
	
	switch (order)
	{
	case 3: p = t3.Find(num); break;
	case 4: p = t4.Find(num); break;
	case 5: p = t5.Find(num); break;
	case 6: p = t6.Find(num); break;
	}
	if (p != nullptr)
	{
		std::cout << "找到关键字" + std::to_string(num) << std::endl;
	}
	else
		std::cout << "未找到" << std::endl;
}

void TreeClear()
{
	switch (order)
	{
	case 3: t3.Clear(); break;
	case 4: t4.Clear(); break;
	case 5: t5.Clear(); break;
	case 6: t6.Clear(); break;
	}
	std::cout << "清除成功！" << std::endl;
}

void ShowHelp()
{
	std::string str =
		" X_Jun(MKXJun)\n"
		" 创建日期：2017-12-1\n"
		" B树测试可用命令：\n"
		" create X           创建X阶B树，X仅允许3-6\n"
		" insert X1 X2...    插入关键字X1, X2...，数目任意，回车或非数字字符结束\n"
		" erase X1 X2...     删除关键字X1, X2...，数目任意，回车或非数字字符结束\n"
		" find X             寻找关键字X\n"
		" show               显示这棵B树\n"
		" clear              清空B树\n"
		" help               重看本帮助\n"
		" quit               退出程序\n";

	std::cout << str;
}
