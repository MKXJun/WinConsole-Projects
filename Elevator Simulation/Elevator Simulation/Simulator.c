#include "Simulator.h"

// 一些前置声明，使得下面的函数可以在这里用到，又不会暴露在主函数中

// 更新电梯
void ElevatorUpdate(_InOut lpElevator pElev);

// 初始化电梯管理系统
RESULT ElevatorSystemInit(_InOut lpElevatorSystem pElevSys, _In int numOfFloors,
	_In ElevatorPropDesc* pArrElevPropDesc, _In int numOfElevs);

// 呼叫一部电梯到该楼层
RESULT ElevatorSystemCall(_InOut lpElevatorSystem pElevSys,_In int floor,_In BOOL isUp);

// 人物相关所要用到的函数以及被作为函数指针传递的函数
void PeopleUpdate(_In Data data);
BOOL PeopleTimeOut(_In Data data);
BOOL PeopleGoingUp(_In Data data);
BOOL PeopleGoingDown(_In Data data);



// 清理缓冲区
void ClearBuffer()
{
	while (getchar() != '\n')
		continue;
}

// 文件读取，跳过一个字符串并读入一个int（范围限制）
RESULT FileSkipWordAndReadInt(_In FILE* fp, _Out int* pNum, _In int minRange, _In int maxRange)
{
	if (fscanf_s(fp, "%*s%d", pNum) <= 0 || *pNum < minRange || *pNum > maxRange)
		return R_ERROR;
	return R_OK;

}

// 屏幕输入，确保输入成功（范围限制）
void ScreenReadInt(_Out int* pNum, _In int minRange, _In int maxRange, _In const char* failNotice)
{
	while (scanf_s("%d", pNum) == 0 || *pNum < minRange || *pNum > maxRange)
	{
		ClearBuffer();
		if (failNotice)
			printf(failNotice);
	}
	ClearBuffer();
}

RESULT SimInit(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;
	memset(pSim, 0, sizeof(Simulator));
	
	// 准备日志输出
	fopen_s(&pSim->fpLog, "log.txt", "w");
	if (!pSim->fpLog)
		return R_ERROR;
	fprintf(pSim->fpLog, "时间点    事件\n");
	return R_OK;
}

RESULT SimInputInit(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;
	SimInit(pSim);
	ElevatorPropDesc epd[6];
	int floors, elevs;
	printf("输入电梯数<1-6>: ");
	ScreenReadInt(&elevs, 1, 6, "重新输入电梯数<1-6>: ");

	printf("输入楼层数<2-40>: ");
	ScreenReadInt(&floors, 2, 40, "重新输入楼层数<2-40>: ");

	printf("输入模拟时间<500-1000000>(1单位100ms): ");
	ScreenReadInt(&pSim->endTime, 500, 1000000, "重新输入模拟时间<500-1000000>(1单位100ms): ");

	printf("输入1单位需要经过的毫秒数<0-200>(1单位100ms): ");
	ScreenReadInt(&pSim->delayTime, 0, 200, "重新输入1单位需要经过的毫秒数<0-200>(1单位100ms): ");

	printf("输入最小容忍等候电梯时间<500-100000>: ");
	ScreenReadInt(&pSim->peopleMinWaitingTime, 500, 100000, "重新输入最小容忍等候电梯时间<500-100000>: ");

	// 最大容忍时间不能比最小容忍时间还小
	printf("输入最大容忍等候电梯时间<%d-100000>: ", pSim->peopleMinWaitingTime);
	while (scanf_s("%d", &pSim->peopleMaxWaitingTime) == 0 || pSim->peopleMaxWaitingTime < 500 || pSim->peopleMaxWaitingTime > 100000
		|| pSim->peopleMaxWaitingTime < pSim->peopleMinWaitingTime)
	{
		ClearBuffer();
		printf("重新输入最大容忍等候电梯时间<%d-100000>: ", pSim->peopleMinWaitingTime);
	}
	ClearBuffer();

	printf("输入电梯静止允许时间<300-10000>: ");
	ScreenReadInt(&epd->staticTime, 300, 10000, "重新输入电梯静止允许时间<300-10000>: ");

	printf("输入电梯上升一层用时<10-100>: ");
	ScreenReadInt(&epd->upTime, 10, 100, "重新输入电梯上升一层用时<10-100>: ");

	printf("输入电梯下降一层用时<10-100>: ");
	ScreenReadInt(&epd->downTime, 10, 100, "重新输入电梯下降一层用时<10-100>: ");

	printf("输入电梯开门用时<10-50>: ");
	ScreenReadInt(&epd->openingTime, 10, 50, "重新输入电梯开门用时<10-50>: ");

	printf("输入电梯关门用时<10-50>: ");
	ScreenReadInt(&epd->closingTime, 10, 50, "重新输入电梯关门用时<10-50>: ");

	printf("输入电梯开门等候用时<30-200>: ");
	ScreenReadInt(&epd->waitingTime, 30, 200, "重新输入电梯开门等候用时<30-200>: ");

	printf("输入电梯最大载荷人数<6-30>: ");
	ScreenReadInt(&epd->maxLoad, 6, 30, "重新输入电梯最大载荷人数<6-30>: ");

	int i;
	for (i = 1; i < elevs; ++i)
		memcpy(epd + i, epd, sizeof(ElevatorPropDesc));
	
	// 初始化电梯
	ElevatorSystemInit(&pSim->elevSys, floors, epd, elevs);
	// 绑定模拟器以支持回溯
	pSim->elevSys.pSim = pSim;

	// 输入每层楼的人数
	system("cls");
	for (i = 0; i <= floors; ++i)
	{
		if (i == 0)
			printf("输入地下层");
		else
			printf("输入%d楼", i);
		printf("人数<0-1000>: ");
		while (scanf_s("%d", pSim->peopleStartCount + i) == 0 || pSim->peopleLeft[i] < 0 || pSim->peopleLeft[i] > 1000)
		{
			ClearBuffer();
			if (i == 0)
				printf("重新输入地下层");
			else
				printf("重新输入%d楼", i);
			printf("人数<0-1000>: ");
		}
		pSim->peopleLeft[i] = pSim->peopleStartCount[i];
		ClearBuffer();
	}
	system("cls");
	return R_OK;
}

RESULT SimSave(_In const Simulator* pSim)
{
	if (!pSim)
		return R_NULLPTR;
	FILE* fp;
	fopen_s(&fp, "sim.txt", "w");
	fprintf(fp, "注意！这句话不能删，确保修改数据时要与冒号隔开至少一格！\n");
	fprintf(fp, "电梯数<1-6>: %d\n", pSim->elevSys.numOfElevators);
	fprintf(fp, "楼层数<2-40>: %d\n", pSim->elevSys.numOfFloors);
	fprintf(fp, "模拟时间<500-1000000>(1单位100ms): %d\n", pSim->endTime);
	fprintf(fp, "1单位时间毫秒数<0-200>(1单位100ms): %d\n", pSim->delayTime);
	fprintf(fp, "最小容忍等待时间数<500-100000>: %d\n", pSim->peopleMinWaitingTime);
	fprintf(fp, "最大容忍等待时间数<Min-100000>: %d\n", pSim->peopleMaxWaitingTime);
	fprintf(fp, "电梯静止时间数<300-10000>: %d\n", pSim->elevSys.Elevators[0].prop.staticTime);
	fprintf(fp, "电梯上升一层用时<10-100>: %d\n", pSim->elevSys.Elevators[0].prop.upTime);
	fprintf(fp, "电梯下降一层用时<10-100>: %d\n", pSim->elevSys.Elevators[0].prop.downTime);
	fprintf(fp, "电梯开门用时<10-50>: %d\n", pSim->elevSys.Elevators[0].prop.openingTime);
	fprintf(fp, "电梯关门用时<10-50>: %d\n", pSim->elevSys.Elevators[0].prop.closingTime);
	fprintf(fp, "电梯开门等候用时<30-200>: %d\n", pSim->elevSys.Elevators[0].prop.waitingTime);
	fprintf(fp, "电梯最大载荷数<6-30>: %d\n\n", pSim->elevSys.Elevators[0].prop.maxLoad);
	
	fprintf(fp, "地下层人数<0-1000>: %d\n", pSim->peopleLeft[0]);
	int i;
	for (i = 1; i <= pSim->elevSys.numOfFloors; ++i)
		fprintf(fp, "%d层人数<0-1000>: %d\n", i, pSim->peopleLeft[i]);
	fclose(fp);
	return R_OK;
}

RESULT SimLoadInit(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;

	FILE* fp;
	fopen_s(&fp, "sim.txt", "r");
	if (!fp)
		return R_NOTFOUND;

	SimInit(pSim);
	ElevatorPropDesc epd[6];
	fscanf_s(fp, "%*s");
	if (FileSkipWordAndReadInt(fp, &pSim->elevSys.numOfElevators, 1, 6) ||
		FileSkipWordAndReadInt(fp, &pSim->elevSys.numOfFloors, 2, 40) ||
		FileSkipWordAndReadInt(fp, &pSim->endTime, 500, 1000000) ||
		FileSkipWordAndReadInt(fp, &pSim->delayTime, 0, 200) ||
		FileSkipWordAndReadInt(fp, &pSim->peopleMinWaitingTime, 500, 100000) ||
		FileSkipWordAndReadInt(fp, &pSim->peopleMaxWaitingTime, 0, 100000) ||
		FileSkipWordAndReadInt(fp, &epd[0].staticTime, 300, 10000) ||
		FileSkipWordAndReadInt(fp, &epd[0].upTime, 10, 100) ||
		FileSkipWordAndReadInt(fp, &epd[0].downTime, 10, 100) ||
		FileSkipWordAndReadInt(fp, &epd[0].openingTime, 10, 50) ||
		FileSkipWordAndReadInt(fp, &epd[0].closingTime, 10, 50) ||
		FileSkipWordAndReadInt(fp, &epd[0].waitingTime, 30, 200) ||
		FileSkipWordAndReadInt(fp, &epd[0].maxLoad, 6, 30))
	{
		fclose(fp);
		return R_ERROR;
	}

	int i;
	for (i = 1; i < pSim->elevSys.numOfElevators; ++i)
		memcpy(epd + i, epd, sizeof(ElevatorPropDesc));

	ElevatorSystemInit(&pSim->elevSys, pSim->elevSys.numOfFloors, epd, pSim->elevSys.numOfElevators);
	// 绑定模拟器以支持回溯
	pSim->elevSys.pSim = pSim;

	for (i = 0; i <= pSim->elevSys.numOfFloors; ++i)
	{
		if (FileSkipWordAndReadInt(fp, &pSim->peopleStartCount[i], 0, 1000) != R_OK)
		{ 
			fclose(fp);
			return R_ERROR; 
		}
		pSim->peopleLeft[i] = pSim->peopleStartCount[i];
	}
		
	fclose(fp);
	return R_OK;
}

RESULT SimUpdate(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;

	int i, j;
	// 所有楼层和电梯都不存在等待的用户，且时间已经到达或超过endTime时结束模拟
	if (pSim->timer >= pSim->endTime)
	{
		for (i = 0; i <= pSim->elevSys.numOfFloors; ++i)
			if (pSim->people[i].length > 0)
				break;
		for (j = 0; j < pSim->elevSys.numOfElevators; ++j)
			if (pSim->elevSys.Elevators[j].currLoad > 0)
				break;
		// 输出最终模拟结果
		if (i > pSim->elevSys.numOfFloors && j >= pSim->elevSys.numOfElevators)
		{
			pSim->isEnd = TRUE;
			fprintf_s(pSim->fpLog, "模拟结束！当前各楼层因不耐烦而离开的人数/总人数如下：\n");
			int floor, peopleTotal = 0, peopleTotalTimeOut = 0;
			for (floor = pSim->elevSys.numOfFloors; floor > 0; --floor)
			{
				fprintf_s(pSim->fpLog, "%02d楼：%d/%d人\n", floor, pSim->peopleTimeOut[floor], pSim->peopleStartCount[floor]);
				peopleTotal += pSim->peopleStartCount[floor];
				peopleTotalTimeOut += pSim->peopleTimeOut[floor];
			}
			fprintf_s(pSim->fpLog, "地下层：%d/%d人\n", pSim->peopleTimeOut[0], pSim->peopleStartCount[floor]);
			fprintf_s(pSim->fpLog, "超时人数/总人数：%d/%d\n", peopleTotalTimeOut, peopleTotal);
			return R_OK;
		}
			
	}
		
	pSim->timer++;
	
	// 增加等候中所有人的currWaitingTime
	for (i = 0; i < 41; ++i)
		ListForeach(&pSim->people[i], PeopleUpdate);
	
	// 对楼层中的所有人进行交互
	for (i = 0; i < 41; ++i)
	{
		Elevator* pElev = NULL;
		BOOL isFound = FALSE;
		// 寻找正在开门或等候的电梯
		for (j = 0; j < pSim->elevSys.numOfElevators; ++j)
		{
			pElev = &pSim->elevSys.Elevators[j];
			// 电梯停在该层，且正在开门或等候
			if (pElev->currFloor == i && (pElev->status & (ELEV_OPENING | ELEV_WAITING)))
			{
				isFound = TRUE;

				if (pElev->status == ELEV_WAITING)
				{
					// 先让电梯里面的人出去
					BOOL hasIn, hasOut;
					Data data;
					// 寻找是否有出去的人
					Node* pNode = pElev->people.front;
					while (pNode && ((Person*)pNode->data.pData)->outFloor != pElev->currFloor)
						pNode = pNode->next;
					if (pNode)
					{
						hasOut = TRUE;
						if (pElev->entryTimer >= 25)
						{
							data = pNode->data;
							ListRemoveFirst(&pElev->people, data);
							pElev->currLoad = pElev->people.length;
							pElev->entryTimer = 1;
							// 日志输出
							fprintf(pSim->fpLog, "%-10d电梯%d[%d楼]: 离开1人  电梯人数:%d\n", 
								pSim->timer, j + 1, pElev->currFloor,  pElev->currLoad);
							continue;
						}
						pElev->entryTimer++;
						continue;
					}
					else
						hasOut = FALSE;

					// 若所有要出去的人已经出去了，这时外面的人可以进来
					// 若电梯之前是要上楼，让想上楼的人进去
					// 若电梯之前是要下楼，让想下楼的人进去
					// 当然前提是电梯没满载
					// 此时按下内部楼层按钮
					if (((pElev->prevStatus == ELEV_GOINGUP && ListFind_If(&pSim->people[i], PeopleGoingUp, &data) == R_OK) ||
						(pElev->prevStatus == ELEV_GOINGDOWN && ListFind_If(&pSim->people[i], PeopleGoingDown, &data) == R_OK)) &&
						pElev->currLoad < pElev->prop.maxLoad)
					{
						hasIn = TRUE;
						pElev->entryTimer++;
						if (pElev->entryTimer >= 25)
						{
							ListPushBack(&pElev->people, data);
							ListBack(&pElev->people, &data);
							ListRemoveFirst(&pSim->people[i], data);
							pElev->currLoad = pElev->people.length;
							pElev->entryTimer = 1;
							pElev->buttonFloor[((Person*)data.pData)->outFloor] = TRUE;
							// 日志输出
							fprintf(pSim->fpLog, "%-10d电梯%d[%d楼]: 进入1人 电梯人数:%d 外部队列人数:%d\n",
								pSim->timer, j + 1, pElev->currFloor, pElev->currLoad, 
								pElev->pElevSys->pSim->people[pElev->currFloor].length);
							fprintf(pSim->fpLog, "%-10d电梯%d[%d楼]: %d楼按钮被按下\n",
								pSim->timer, j + 1, pElev->currFloor, ((Person*)data.pData)->outFloor);
						}
						continue;
					}
					else
						hasIn = FALSE;

					if (!hasIn && !hasOut)
						pElev->entryTimer = 0;
				}
			}
		}

		// 如果没找到，将所有等的不耐烦的人从队列中踢掉，正在等候的人则按下上楼或下楼按钮
		if (!isFound)
		{
			int prevLength = pSim->people[i].length;
			ListRemove_If(&pSim->people[i], PeopleTimeOut);
			pSim->peopleTimeOut[i] += prevLength - pSim->people[i].length;
			// 日志输出
			if (prevLength - pSim->people[i].length > 0)
			{
				fprintf(pSim->fpLog, "%-10d[%d楼]: 1人不耐烦而离开 当前楼层已有%d人离开\n",
					pSim->timer, i, pElev->pElevSys->pSim->peopleTimeOut[i]);
			}
			// 寻找是否有上楼或下楼的用户，且按钮此时没被按下，有则按下按钮
			// 若队首的人等待时间超过100单位时间，系统再次寻找一部电梯叫过来
			Data data;
			if (ListFind_If(&pSim->people[i], PeopleGoingUp, &data) == R_OK)
			{
				if (pSim->elevSys.buttonUp[i] == FALSE)
				{
					pSim->elevSys.buttonUp[i] = TRUE;
					ElevatorSystemCall(&pSim->elevSys, i, TRUE);

					// 日志输出
					fprintf(pSim->fpLog, "%-10d[%d楼]: 电梯外层上升按钮被按下\n",
						pSim->timer, j + 1);
				}
				else if (((Person*)data.pData)->currWaitingTime % 300 == 0)
					ElevatorSystemCall(&pSim->elevSys, i, TRUE);
			}
			if (ListFind_If(&pSim->people[i], PeopleGoingDown, &data) == R_OK)
			{
				if (pSim->elevSys.buttonDown[i] == FALSE)
				{
					pSim->elevSys.buttonDown[i] = TRUE;
					ElevatorSystemCall(&pSim->elevSys, i, FALSE);

					// 日志输出
					fprintf(pSim->fpLog, "%-10d[%d楼]: 电梯外层下降按钮被按下\n",
						pSim->timer, j + 1);
				}
				else if (((Person*)data.pData)->currWaitingTime % 300 == 0)
					ElevatorSystemCall(&pSim->elevSys, i, FALSE);
			}
				
		}
	}

	// 对所有电梯进行更新
	for (i = 0; i < pSim->elevSys.numOfElevators; ++i)
		ElevatorUpdate(&pSim->elevSys.Elevators[i]);

	return R_OK;
}

RESULT SimPeopleRandomGenerator(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;

	int i;
	if (pSim->endTime <= pSim->timer)
		return R_OK;


	//for (i = 2; i <= pSim->elevSys.numOfFloors; ++i)
	//{
	//	// 越高楼层的人会在越早的一个时间段出发
	//	float div = (float)pSim->endTime / (pSim->elevSys.numOfFloors);	// 切分时间区间
	//	int left = (int)(div * (pSim->elevSys.numOfFloors - i));
	//	int right = (int)(div * (pSim->elevSys.numOfFloors - i + 2));
	//	if (pSim->timer >= left && pSim->timer < right && rand() % (right - pSim->timer) < pSim->peopleLeft[i])
	//	{
	//		Person person;
	//		memset(&person, 0, sizeof person);

	//		if (pSim->peopleMaxWaitingTime == pSim->peopleMinWaitingTime)
	//			person.giveUpTime = pSim->peopleMinWaitingTime;
	//		else
	//			person.giveUpTime = pSim->peopleMinWaitingTime + rand() % (pSim->peopleMaxWaitingTime - pSim->peopleMinWaitingTime);
	//		person.inFloor = i;
	//		person.outFloor = 1;//rand() % (pSim->elevSys.numOfFloors + 1);
	//		while (person.outFloor == i)
	//			person.outFloor = rand() % (pSim->elevSys.numOfFloors + 1);
	//		Data data = { &person, sizeof person };
	//		ListPushBack(&pSim->people[i], data);
	//		pSim->peopleLeft[i]--;

	//		// 日志输出
	//		fprintf(pSim->fpLog, "%-10d[%d楼]: 队列人数+1， 当前人数：%d\n",
	//			pSim->timer, i, pSim->people[i].length);
	//	}
	//}

	// 所有时刻都有相等的概率会往等待队列添加一个人，然后这个人将按下电梯按钮
	for (i = 0; i < 41; ++i)
	{
		if (rand() % (pSim->endTime - pSim->timer) < pSim->peopleLeft[i])
		{
			Person person;
			memset(&person, 0, sizeof person);

			if (pSim->peopleMaxWaitingTime == pSim->peopleMinWaitingTime)
				person.giveUpTime = pSim->peopleMinWaitingTime;
			else
				person.giveUpTime = pSim->peopleMinWaitingTime + rand() % (pSim->peopleMaxWaitingTime - pSim->peopleMinWaitingTime);
			person.inFloor = i;
			person.outFloor = rand() % (pSim->elevSys.numOfFloors + 1);
			while (person.outFloor == i)
				person.outFloor = rand() % (pSim->elevSys.numOfFloors + 1);
			Data data = { &person, sizeof person };
			ListPushBack(&pSim->people[i], data);
			pSim->peopleLeft[i]--;

			// 日志输出
			fprintf(pSim->fpLog, "%-10d[%d楼]: 队列人数+1， 当前人数：%d\n",
				pSim->timer, i, pSim->people[i].length);
		}
	}
	return R_OK;
}

RESULT SimRealTimeElevShow(_In HANDLE hOut, _In Simulator * pSim)
{
	if (!pSim || !hOut)
		return R_NULLPTR;


	COORD pos = { 10, 0 };
	int i;
	if (pSim->isEnd)
	{
		pos.X = 0; pos.Y = 6;
		SetConsoleCursorPosition(hOut, pos);
		printf("模拟结束，按回车退出程序");
	}

	if (pSim->timer <= 1)
	{
		pos.X = 10; pos.Y = 0;
		SetConsoleCursorPosition(hOut, pos);
		// 输出主干部分
		printf(
			"电梯实时模拟系统                                                      ↑↓等候  超时  剩余\n\n"
			"电梯数:\n楼层数:\n模拟时间:\n帧间隔:\n");
		// 输出电梯信息UI部分（6部电梯分成2行3列来显示）
		for (i = 0; i < pSim->elevSys.numOfElevators; ++i)
		{
			pos.X = 16 * i % 48;
			pos.Y = i >= 3 ? 14 : 8;
			SetConsoleCursorPosition(hOut, pos);
			printf("电梯%d", i + 1);
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("楼层: ");
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("状态: ");
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("载荷: ");
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("计时: ");
			pos.X = 56 + 4 * i; pos.Y = 0;
			SetConsoleCursorPosition(hOut, pos);
			printf("E%d", i + 1);
		}
		// 输出电梯楼层
		int floor;
		pos.X = 50;
		for (floor = pSim->elevSys.numOfFloors; floor > 0; --floor)
		{
			pos.Y = pSim->elevSys.numOfFloors - floor + 1;
			SetConsoleCursorPosition(hOut, pos);
			printf("F%d", floor);
		}
		pos.Y = pSim->elevSys.numOfFloors + 1;
		SetConsoleCursorPosition(hOut, pos);
		printf("B1");
	}

	if (pSim->timer >= 1)
	{
		// 输出模拟数据
		// 电梯数
		pos.X = 8; pos.Y = 2;
		SetConsoleCursorPosition(hOut, pos);
		printf("%d", pSim->elevSys.numOfElevators);
		// 楼层数
		pos.Y++;
		SetConsoleCursorPosition(hOut, pos);
		printf("%d", pSim->elevSys.numOfFloors);
		// 模拟时间
		pos.Y++;
		pos.X = 10;
		SetConsoleCursorPosition(hOut, pos);
		int len = printf("%d/%d", pSim->timer, pSim->endTime);
		printf("%*s", 20 - len, "");
		// 速度
		pos.Y++;
		pos.X = 8;
		SetConsoleCursorPosition(hOut, pos);
		printf("%-4d", pSim->delayTime);

		// 输出电梯具体数据（6部电梯分成2行3列来显示）
		for (i = 0; i < pSim->elevSys.numOfElevators; ++i)
		{
			lpElevator pElev = &pSim->elevSys.Elevators[i];
			// 电梯所在楼层
			pos.X = 6 + 16 * i % 48;
			pos.Y = i >= 3 ? 15 : 9;
			SetConsoleCursorPosition(hOut, pos);
			printf("%-10d", pElev->currFloor);
			// 电梯状态
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);

			int endTime;
			switch (pElev->status)
			{
			case ELEV_OFF:	printf("%-10s", "关闭"); endTime = pElev->prop.closingTime; break;
			case ELEV_IDLE:	printf("%-10s", "空闲"); endTime = pElev->prop.staticTime; break;
			case ELEV_OPENING:	printf("%-10s", "开门中"); endTime = pElev->prop.openingTime; break;
			case ELEV_WAITING:	printf("%-10s", "等候中"); endTime = pElev->prop.waitingTime; break;
			case ELEV_CLOSING:	printf("%-10s", "关门中"); endTime = pElev->prop.closingTime; break;
			case ELEV_GOINGUP:	printf("%-10s", "上升"); endTime = pElev->prop.upTime; break;
			case ELEV_GOINGDOWN:	printf("%-10s", "下降"); endTime = pElev->prop.downTime; break;
			}
			// 电梯载荷
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			int len = printf("%d/%d", pElev->currLoad, pElev->prop.maxLoad);
			printf("%*s", 10 - len, "");
			// 电梯计时器
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			len = printf("%d/%d", pElev->timer, endTime);
			printf("%*s", 10 - len, "");
			// 电梯内部按钮与当前楼层显示
			int floor;
			pos.X = 55 + i * 4;
			for (floor = pSim->elevSys.numOfFloors; floor >= 0; --floor)
			{
				pos.Y = pSim->elevSys.numOfFloors - floor + 1;
				SetConsoleCursorPosition(hOut, pos);
				if (pElev->buttonFloor[floor] == TRUE)
					printf(" XX ");
				else
					printf("    ");
			}
			pos.Y = pSim->elevSys.numOfFloors - pElev->currFloor + 1;
			SetConsoleCursorPosition(hOut, pos);
			printf("[%02d]", pElev->currLoad);
		}
		// 输出楼层信息
		int floor;
		pos.X = 80;
		for (floor = pSim->elevSys.numOfFloors; floor >= 0; --floor)
		{
			pos.Y = pSim->elevSys.numOfFloors - floor + 1;
			SetConsoleCursorPosition(hOut, pos);
			printf("%-2s%-2s%4d%6d%6d", pSim->elevSys.buttonUp[floor] ? "□" : "  ",
				pSim->elevSys.buttonDown[floor] ? "□" : "  ",
				pSim->people[floor].length, pSim->peopleTimeOut[floor], pSim->peopleLeft[floor]);
		}	
	}
	return R_OK;
}

RESULT SimClose(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;

	if (pSim->fpLog)
	{
		fclose(pSim->fpLog);
		pSim->fpLog = NULL;
	}
	return R_OK;
}
