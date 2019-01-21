#include "Simulator.h"

// һЩǰ��������ʹ������ĺ��������������õ����ֲ��ᱩ¶����������

// ���µ���
void ElevatorUpdate(_InOut lpElevator pElev);

// ��ʼ�����ݹ���ϵͳ
RESULT ElevatorSystemInit(_InOut lpElevatorSystem pElevSys, _In int numOfFloors,
	_In ElevatorPropDesc* pArrElevPropDesc, _In int numOfElevs);

// ����һ�����ݵ���¥��
RESULT ElevatorSystemCall(_InOut lpElevatorSystem pElevSys,_In int floor,_In BOOL isUp);

// ���������Ҫ�õ��ĺ����Լ�����Ϊ����ָ�봫�ݵĺ���
void PeopleUpdate(_In Data data);
BOOL PeopleTimeOut(_In Data data);
BOOL PeopleGoingUp(_In Data data);
BOOL PeopleGoingDown(_In Data data);



// ��������
void ClearBuffer()
{
	while (getchar() != '\n')
		continue;
}

// �ļ���ȡ������һ���ַ���������һ��int����Χ���ƣ�
RESULT FileSkipWordAndReadInt(_In FILE* fp, _Out int* pNum, _In int minRange, _In int maxRange)
{
	if (fscanf_s(fp, "%*s%d", pNum) <= 0 || *pNum < minRange || *pNum > maxRange)
		return R_ERROR;
	return R_OK;

}

// ��Ļ���룬ȷ������ɹ�����Χ���ƣ�
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
	
	// ׼����־���
	fopen_s(&pSim->fpLog, "log.txt", "w");
	if (!pSim->fpLog)
		return R_ERROR;
	fprintf(pSim->fpLog, "ʱ���    �¼�\n");
	return R_OK;
}

RESULT SimInputInit(_InOut Simulator * pSim)
{
	if (!pSim)
		return R_NULLPTR;
	SimInit(pSim);
	ElevatorPropDesc epd[6];
	int floors, elevs;
	printf("���������<1-6>: ");
	ScreenReadInt(&elevs, 1, 6, "�������������<1-6>: ");

	printf("����¥����<2-40>: ");
	ScreenReadInt(&floors, 2, 40, "��������¥����<2-40>: ");

	printf("����ģ��ʱ��<500-1000000>(1��λ100ms): ");
	ScreenReadInt(&pSim->endTime, 500, 1000000, "��������ģ��ʱ��<500-1000000>(1��λ100ms): ");

	printf("����1��λ��Ҫ�����ĺ�����<0-200>(1��λ100ms): ");
	ScreenReadInt(&pSim->delayTime, 0, 200, "��������1��λ��Ҫ�����ĺ�����<0-200>(1��λ100ms): ");

	printf("������С���̵Ⱥ����ʱ��<500-100000>: ");
	ScreenReadInt(&pSim->peopleMinWaitingTime, 500, 100000, "����������С���̵Ⱥ����ʱ��<500-100000>: ");

	// �������ʱ�䲻�ܱ���С����ʱ�仹С
	printf("����������̵Ⱥ����ʱ��<%d-100000>: ", pSim->peopleMinWaitingTime);
	while (scanf_s("%d", &pSim->peopleMaxWaitingTime) == 0 || pSim->peopleMaxWaitingTime < 500 || pSim->peopleMaxWaitingTime > 100000
		|| pSim->peopleMaxWaitingTime < pSim->peopleMinWaitingTime)
	{
		ClearBuffer();
		printf("��������������̵Ⱥ����ʱ��<%d-100000>: ", pSim->peopleMinWaitingTime);
	}
	ClearBuffer();

	printf("������ݾ�ֹ����ʱ��<300-10000>: ");
	ScreenReadInt(&epd->staticTime, 300, 10000, "����������ݾ�ֹ����ʱ��<300-10000>: ");

	printf("�����������һ����ʱ<10-100>: ");
	ScreenReadInt(&epd->upTime, 10, 100, "���������������һ����ʱ<10-100>: ");

	printf("��������½�һ����ʱ<10-100>: ");
	ScreenReadInt(&epd->downTime, 10, 100, "������������½�һ����ʱ<10-100>: ");

	printf("������ݿ�����ʱ<10-50>: ");
	ScreenReadInt(&epd->openingTime, 10, 50, "����������ݿ�����ʱ<10-50>: ");

	printf("������ݹ�����ʱ<10-50>: ");
	ScreenReadInt(&epd->closingTime, 10, 50, "����������ݹ�����ʱ<10-50>: ");

	printf("������ݿ��ŵȺ���ʱ<30-200>: ");
	ScreenReadInt(&epd->waitingTime, 30, 200, "����������ݿ��ŵȺ���ʱ<30-200>: ");

	printf("�����������غ�����<6-30>: ");
	ScreenReadInt(&epd->maxLoad, 6, 30, "���������������غ�����<6-30>: ");

	int i;
	for (i = 1; i < elevs; ++i)
		memcpy(epd + i, epd, sizeof(ElevatorPropDesc));
	
	// ��ʼ������
	ElevatorSystemInit(&pSim->elevSys, floors, epd, elevs);
	// ��ģ������֧�ֻ���
	pSim->elevSys.pSim = pSim;

	// ����ÿ��¥������
	system("cls");
	for (i = 0; i <= floors; ++i)
	{
		if (i == 0)
			printf("������²�");
		else
			printf("����%d¥", i);
		printf("����<0-1000>: ");
		while (scanf_s("%d", pSim->peopleStartCount + i) == 0 || pSim->peopleLeft[i] < 0 || pSim->peopleLeft[i] > 1000)
		{
			ClearBuffer();
			if (i == 0)
				printf("����������²�");
			else
				printf("��������%d¥", i);
			printf("����<0-1000>: ");
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
	fprintf(fp, "ע�⣡��仰����ɾ��ȷ���޸�����ʱҪ��ð�Ÿ�������һ��\n");
	fprintf(fp, "������<1-6>: %d\n", pSim->elevSys.numOfElevators);
	fprintf(fp, "¥����<2-40>: %d\n", pSim->elevSys.numOfFloors);
	fprintf(fp, "ģ��ʱ��<500-1000000>(1��λ100ms): %d\n", pSim->endTime);
	fprintf(fp, "1��λʱ�������<0-200>(1��λ100ms): %d\n", pSim->delayTime);
	fprintf(fp, "��С���̵ȴ�ʱ����<500-100000>: %d\n", pSim->peopleMinWaitingTime);
	fprintf(fp, "������̵ȴ�ʱ����<Min-100000>: %d\n", pSim->peopleMaxWaitingTime);
	fprintf(fp, "���ݾ�ֹʱ����<300-10000>: %d\n", pSim->elevSys.Elevators[0].prop.staticTime);
	fprintf(fp, "��������һ����ʱ<10-100>: %d\n", pSim->elevSys.Elevators[0].prop.upTime);
	fprintf(fp, "�����½�һ����ʱ<10-100>: %d\n", pSim->elevSys.Elevators[0].prop.downTime);
	fprintf(fp, "���ݿ�����ʱ<10-50>: %d\n", pSim->elevSys.Elevators[0].prop.openingTime);
	fprintf(fp, "���ݹ�����ʱ<10-50>: %d\n", pSim->elevSys.Elevators[0].prop.closingTime);
	fprintf(fp, "���ݿ��ŵȺ���ʱ<30-200>: %d\n", pSim->elevSys.Elevators[0].prop.waitingTime);
	fprintf(fp, "��������غ���<6-30>: %d\n\n", pSim->elevSys.Elevators[0].prop.maxLoad);
	
	fprintf(fp, "���²�����<0-1000>: %d\n", pSim->peopleLeft[0]);
	int i;
	for (i = 1; i <= pSim->elevSys.numOfFloors; ++i)
		fprintf(fp, "%d������<0-1000>: %d\n", i, pSim->peopleLeft[i]);
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
	// ��ģ������֧�ֻ���
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
	// ����¥��͵��ݶ������ڵȴ����û�����ʱ���Ѿ�����򳬹�endTimeʱ����ģ��
	if (pSim->timer >= pSim->endTime)
	{
		for (i = 0; i <= pSim->elevSys.numOfFloors; ++i)
			if (pSim->people[i].length > 0)
				break;
		for (j = 0; j < pSim->elevSys.numOfElevators; ++j)
			if (pSim->elevSys.Elevators[j].currLoad > 0)
				break;
		// �������ģ����
		if (i > pSim->elevSys.numOfFloors && j >= pSim->elevSys.numOfElevators)
		{
			pSim->isEnd = TRUE;
			fprintf_s(pSim->fpLog, "ģ���������ǰ��¥�����ͷ����뿪������/���������£�\n");
			int floor, peopleTotal = 0, peopleTotalTimeOut = 0;
			for (floor = pSim->elevSys.numOfFloors; floor > 0; --floor)
			{
				fprintf_s(pSim->fpLog, "%02d¥��%d/%d��\n", floor, pSim->peopleTimeOut[floor], pSim->peopleStartCount[floor]);
				peopleTotal += pSim->peopleStartCount[floor];
				peopleTotalTimeOut += pSim->peopleTimeOut[floor];
			}
			fprintf_s(pSim->fpLog, "���²㣺%d/%d��\n", pSim->peopleTimeOut[0], pSim->peopleStartCount[floor]);
			fprintf_s(pSim->fpLog, "��ʱ����/��������%d/%d\n", peopleTotalTimeOut, peopleTotal);
			return R_OK;
		}
			
	}
		
	pSim->timer++;
	
	// ���ӵȺ��������˵�currWaitingTime
	for (i = 0; i < 41; ++i)
		ListForeach(&pSim->people[i], PeopleUpdate);
	
	// ��¥���е������˽��н���
	for (i = 0; i < 41; ++i)
	{
		Elevator* pElev = NULL;
		BOOL isFound = FALSE;
		// Ѱ�����ڿ��Ż�Ⱥ�ĵ���
		for (j = 0; j < pSim->elevSys.numOfElevators; ++j)
		{
			pElev = &pSim->elevSys.Elevators[j];
			// ����ͣ�ڸò㣬�����ڿ��Ż�Ⱥ�
			if (pElev->currFloor == i && (pElev->status & (ELEV_OPENING | ELEV_WAITING)))
			{
				isFound = TRUE;

				if (pElev->status == ELEV_WAITING)
				{
					// ���õ���������˳�ȥ
					BOOL hasIn, hasOut;
					Data data;
					// Ѱ���Ƿ��г�ȥ����
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
							// ��־���
							fprintf(pSim->fpLog, "%-10d����%d[%d¥]: �뿪1��  ��������:%d\n", 
								pSim->timer, j + 1, pElev->currFloor,  pElev->currLoad);
							continue;
						}
						pElev->entryTimer++;
						continue;
					}
					else
						hasOut = FALSE;

					// ������Ҫ��ȥ�����Ѿ���ȥ�ˣ���ʱ������˿��Խ���
					// ������֮ǰ��Ҫ��¥��������¥���˽�ȥ
					// ������֮ǰ��Ҫ��¥��������¥���˽�ȥ
					// ��Ȼǰ���ǵ���û����
					// ��ʱ�����ڲ�¥�㰴ť
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
							// ��־���
							fprintf(pSim->fpLog, "%-10d����%d[%d¥]: ����1�� ��������:%d �ⲿ��������:%d\n",
								pSim->timer, j + 1, pElev->currFloor, pElev->currLoad, 
								pElev->pElevSys->pSim->people[pElev->currFloor].length);
							fprintf(pSim->fpLog, "%-10d����%d[%d¥]: %d¥��ť������\n",
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

		// ���û�ҵ��������еȵĲ��ͷ����˴Ӷ������ߵ������ڵȺ����������¥����¥��ť
		if (!isFound)
		{
			int prevLength = pSim->people[i].length;
			ListRemove_If(&pSim->people[i], PeopleTimeOut);
			pSim->peopleTimeOut[i] += prevLength - pSim->people[i].length;
			// ��־���
			if (prevLength - pSim->people[i].length > 0)
			{
				fprintf(pSim->fpLog, "%-10d[%d¥]: 1�˲��ͷ����뿪 ��ǰ¥������%d���뿪\n",
					pSim->timer, i, pElev->pElevSys->pSim->peopleTimeOut[i]);
			}
			// Ѱ���Ƿ�����¥����¥���û����Ұ�ť��ʱû�����£������°�ť
			// �����׵��˵ȴ�ʱ�䳬��100��λʱ�䣬ϵͳ�ٴ�Ѱ��һ�����ݽй���
			Data data;
			if (ListFind_If(&pSim->people[i], PeopleGoingUp, &data) == R_OK)
			{
				if (pSim->elevSys.buttonUp[i] == FALSE)
				{
					pSim->elevSys.buttonUp[i] = TRUE;
					ElevatorSystemCall(&pSim->elevSys, i, TRUE);

					// ��־���
					fprintf(pSim->fpLog, "%-10d[%d¥]: �������������ť������\n",
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

					// ��־���
					fprintf(pSim->fpLog, "%-10d[%d¥]: ��������½���ť������\n",
						pSim->timer, j + 1);
				}
				else if (((Person*)data.pData)->currWaitingTime % 300 == 0)
					ElevatorSystemCall(&pSim->elevSys, i, FALSE);
			}
				
		}
	}

	// �����е��ݽ��и���
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
	//	// Խ��¥����˻���Խ���һ��ʱ��γ���
	//	float div = (float)pSim->endTime / (pSim->elevSys.numOfFloors);	// �з�ʱ������
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

	//		// ��־���
	//		fprintf(pSim->fpLog, "%-10d[%d¥]: ��������+1�� ��ǰ������%d\n",
	//			pSim->timer, i, pSim->people[i].length);
	//	}
	//}

	// ����ʱ�̶�����ȵĸ��ʻ����ȴ��������һ���ˣ�Ȼ������˽����µ��ݰ�ť
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

			// ��־���
			fprintf(pSim->fpLog, "%-10d[%d¥]: ��������+1�� ��ǰ������%d\n",
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
		printf("ģ����������س��˳�����");
	}

	if (pSim->timer <= 1)
	{
		pos.X = 10; pos.Y = 0;
		SetConsoleCursorPosition(hOut, pos);
		// ������ɲ���
		printf(
			"����ʵʱģ��ϵͳ                                                      �����Ⱥ�  ��ʱ  ʣ��\n\n"
			"������:\n¥����:\nģ��ʱ��:\n֡���:\n");
		// ���������ϢUI���֣�6�����ݷֳ�2��3������ʾ��
		for (i = 0; i < pSim->elevSys.numOfElevators; ++i)
		{
			pos.X = 16 * i % 48;
			pos.Y = i >= 3 ? 14 : 8;
			SetConsoleCursorPosition(hOut, pos);
			printf("����%d", i + 1);
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("¥��: ");
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("״̬: ");
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("�غ�: ");
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			printf("��ʱ: ");
			pos.X = 56 + 4 * i; pos.Y = 0;
			SetConsoleCursorPosition(hOut, pos);
			printf("E%d", i + 1);
		}
		// �������¥��
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
		// ���ģ������
		// ������
		pos.X = 8; pos.Y = 2;
		SetConsoleCursorPosition(hOut, pos);
		printf("%d", pSim->elevSys.numOfElevators);
		// ¥����
		pos.Y++;
		SetConsoleCursorPosition(hOut, pos);
		printf("%d", pSim->elevSys.numOfFloors);
		// ģ��ʱ��
		pos.Y++;
		pos.X = 10;
		SetConsoleCursorPosition(hOut, pos);
		int len = printf("%d/%d", pSim->timer, pSim->endTime);
		printf("%*s", 20 - len, "");
		// �ٶ�
		pos.Y++;
		pos.X = 8;
		SetConsoleCursorPosition(hOut, pos);
		printf("%-4d", pSim->delayTime);

		// ������ݾ������ݣ�6�����ݷֳ�2��3������ʾ��
		for (i = 0; i < pSim->elevSys.numOfElevators; ++i)
		{
			lpElevator pElev = &pSim->elevSys.Elevators[i];
			// ��������¥��
			pos.X = 6 + 16 * i % 48;
			pos.Y = i >= 3 ? 15 : 9;
			SetConsoleCursorPosition(hOut, pos);
			printf("%-10d", pElev->currFloor);
			// ����״̬
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);

			int endTime;
			switch (pElev->status)
			{
			case ELEV_OFF:	printf("%-10s", "�ر�"); endTime = pElev->prop.closingTime; break;
			case ELEV_IDLE:	printf("%-10s", "����"); endTime = pElev->prop.staticTime; break;
			case ELEV_OPENING:	printf("%-10s", "������"); endTime = pElev->prop.openingTime; break;
			case ELEV_WAITING:	printf("%-10s", "�Ⱥ���"); endTime = pElev->prop.waitingTime; break;
			case ELEV_CLOSING:	printf("%-10s", "������"); endTime = pElev->prop.closingTime; break;
			case ELEV_GOINGUP:	printf("%-10s", "����"); endTime = pElev->prop.upTime; break;
			case ELEV_GOINGDOWN:	printf("%-10s", "�½�"); endTime = pElev->prop.downTime; break;
			}
			// �����غ�
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			int len = printf("%d/%d", pElev->currLoad, pElev->prop.maxLoad);
			printf("%*s", 10 - len, "");
			// ���ݼ�ʱ��
			pos.Y++;
			SetConsoleCursorPosition(hOut, pos);
			len = printf("%d/%d", pElev->timer, endTime);
			printf("%*s", 10 - len, "");
			// �����ڲ���ť�뵱ǰ¥����ʾ
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
		// ���¥����Ϣ
		int floor;
		pos.X = 80;
		for (floor = pSim->elevSys.numOfFloors; floor >= 0; --floor)
		{
			pos.Y = pSim->elevSys.numOfFloors - floor + 1;
			SetConsoleCursorPosition(hOut, pos);
			printf("%-2s%-2s%4d%6d%6d", pSim->elevSys.buttonUp[floor] ? "��" : "  ",
				pSim->elevSys.buttonDown[floor] ? "��" : "  ",
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
