#include "Simulator.h"
#include <stdio.h>
#include <crtdbg.h>
#include <time.h>
#include <Windows.h>


void ShowAuthor();

int main()
{
	system("color f0");
	// 初始化窗口
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD bfSize = { 110, 50 };
	SMALL_RECT rc = { 0, 0, 109, 44 };
	CONSOLE_CURSOR_INFO cci = { 1, FALSE };
	SetConsoleScreenBufferSize(hOut, bfSize);
	SetConsoleWindowInfo(hOut, TRUE, &rc);

	srand((unsigned)time(0));

	// 显示作者信息
	ShowAuthor();
	// 初始化模拟器
	Simulator sim;
	FILE* fp;
	fopen_s(&fp, "sim.txt", "r");
	if (!fp)
	{
		printf("找不到文件sim.txt！请手动输入以初始化模拟器。\n");
		SimInputInit(&sim);
		SimSave(&sim);
	}
	else if (SimLoadInit(&sim) == R_ERROR)
	{
		fclose(fp);
		printf("文件sim.txt数据格式有误！请手动输入以初始化模拟器。\n");
		SimInputInit(&sim);
		SimSave(&sim);
	}
	// 隐藏光标
	SetConsoleCursorInfo(hOut, &cci);

	while (sim.isEnd == FALSE)
	{
		if (sim.delayTime > 0)
			Sleep(sim.delayTime);
		SimPeopleRandomGenerator(&sim);
		SimUpdate(&sim); 
		SimRealTimeElevShow(hOut, &sim);
	}
	
	SimClose(&sim);
	CloseHandle(hOut);
	_CrtDumpMemoryLeaks();
	getchar();
}

void ShowAuthor()
{
	printf("MKXJun(X_Jun)\n");
	printf("创建日期：2017-12-6\n");
	printf("若之前未运行过该程序，您需要手动输入基本参数。\n");
	printf("如果当期文件夹存在sim.txt，则将进入自动模拟。\n");
	printf("您可以修改sim.txt来省去繁杂的输入步骤。\n");
	printf("或者您也可以删掉sim.txt来重新手动输入。\n");
	printf("按回车键继续。");
	getchar();
	system("cls");
}
