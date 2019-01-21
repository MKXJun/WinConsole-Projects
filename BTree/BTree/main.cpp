#include "BTree.h"
#include <iostream>
#include <functional>
void Parser(std::string str);

int main()
{
	system("color f0");
	Parser("help");
	int n = 5;
	BTree<int, int, 3> b;
	std::string str;
	while (std::getline(std::cin, str))
		Parser(str);
}
