#include "Simulator.h"
#include <stdio.h>
#include <crtdbg.h>
#include <time.h>
#include <Windows.h>


void ShowAuthor();

int main()
{
	system("color f0");
	// ��ʼ������
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD bfSize = { 110, 50 };
	SMALL_RECT rc = { 0, 0, 109, 44 };
	CONSOLE_CURSOR_INFO cci = { 1, FALSE };
	SetConsoleScreenBufferSize(hOut, bfSize);
	SetConsoleWindowInfo(hOut, TRUE, &rc);

	srand((unsigned)time(0));

	// ��ʾ������Ϣ
	ShowAuthor();
	// ��ʼ��ģ����
	Simulator sim;
	FILE* fp;
	fopen_s(&fp, "sim.txt", "r");
	if (!fp)
	{
		printf("�Ҳ����ļ�sim.txt�����ֶ������Գ�ʼ��ģ������\n");
		SimInputInit(&sim);
		SimSave(&sim);
	}
	else if (SimLoadInit(&sim) == R_ERROR)
	{
		fclose(fp);
		printf("�ļ�sim.txt���ݸ�ʽ�������ֶ������Գ�ʼ��ģ������\n");
		SimInputInit(&sim);
		SimSave(&sim);
	}
	// ���ع��
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
	printf("�������ڣ�2017-12-6\n");
	printf("��֮ǰδ���й��ó�������Ҫ�ֶ��������������\n");
	printf("��������ļ��д���sim.txt���򽫽����Զ�ģ�⡣\n");
	printf("�������޸�sim.txt��ʡȥ���ӵ����벽�衣\n");
	printf("������Ҳ����ɾ��sim.txt�������ֶ����롣\n");
	printf("���س���������");
	getchar();
	system("cls");
}
