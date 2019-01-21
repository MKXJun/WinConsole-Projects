#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include <stdio.h>
#include "ElevatorSystem.h"

// ģ����
typedef struct Simulator
{
	ElevatorSystem  elevSys;
	List            people[41];	// �������40��¥
	int			    timer;		// ��ʱ��
	int				endTime;	// ����ʱ��
	int				delayTime;	// ֡�ӳ�ʱ��
	BOOL			isEnd;		// ģ���Ƿ����


	int				peopleMinWaitingTime;	// ����С���̵ȴ�ʱ��
	int				peopleMaxWaitingTime;	// ��������̵ȴ�ʱ��
	int				peopleLeft[41];			// ʣ������
	int				peopleTimeOut[41];		// ��ʱ����
	int				peopleStartCount[41];	// ��ʼ����

	FILE*			fpLog;					// ��־�ļ�ָ��
} Simulator;

// �����������ʽ����ʼ��ģ����
RESULT SimInputInit(_InOut Simulator* pSim);

// ��ȡsim.txt��������ʼ��ģ����
RESULT SimLoadInit(_InOut Simulator* pSim);

// ������������ݵ��ļ�
RESULT SimSave(_In const Simulator* pSim);

// ���º���
RESULT SimUpdate(_InOut Simulator* pSim);

// ��Ա���������
RESULT SimPeopleRandomGenerator(_InOut Simulator* pSim);

// ʵʱ��ʾ����ģ������
RESULT SimRealTimeElevShow(_In HANDLE hOut, _In Simulator* pSim);

// �ر�ģ����
RESULT SimClose(_InOut Simulator* pSim);





#endif
