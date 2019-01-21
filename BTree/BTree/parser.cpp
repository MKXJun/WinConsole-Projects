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
		puts("δ����B��������ʹ��create���");
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
		puts("δ֪�������help�鿴������");
	}
	putchar('>');
	SetConsoleTextAttribute(handle, 0xF4);
}

void TreeCreate(std::istringstream& iss)
{
	iss >> order;
	if (order < 3 || order > 6)
	{
		puts("���Ϸ�������");
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
		puts("�����ɹ���");
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
	std::cout << "�ɹ�����" + std::to_string(count) + "��Ԫ�أ�" << std::endl;
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
	std::cout << "�ɹ�ɾ��" + std::to_string(count) + "��Ԫ�أ�" << std::endl;
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
		std::cout << "�����ʽ����" << std::endl;
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
		std::cout << "�ҵ��ؼ���" + std::to_string(num) << std::endl;
	}
	else
		std::cout << "δ�ҵ�" << std::endl;
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
	std::cout << "����ɹ���" << std::endl;
}

void ShowHelp()
{
	std::string str =
		" X_Jun(MKXJun)\n"
		" �������ڣ�2017-12-1\n"
		" B�����Կ������\n"
		" create X           ����X��B����X������3-6\n"
		" insert X1 X2...    ����ؼ���X1, X2...����Ŀ���⣬�س���������ַ�����\n"
		" erase X1 X2...     ɾ���ؼ���X1, X2...����Ŀ���⣬�س���������ַ�����\n"
		" find X             Ѱ�ҹؼ���X\n"
		" show               ��ʾ���B��\n"
		" clear              ���B��\n"
		" help               �ؿ�������\n"
		" quit               �˳�����\n";

	std::cout << str;
}
