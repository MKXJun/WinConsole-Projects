#include "ElevatorSystem.h"
#include "Simulator.h"

RESULT ElevatorSystemInit(_InOut lpElevatorSystem pElevSys, _In int numOfFloors,
	_In const ElevatorPropDesc* pArrElevPropDesc, _In int numOfElevs)
{
	if (!pElevSys || !pArrElevPropDesc)
		return R_NULLPTR;

	memset(pElevSys, 0, sizeof(ElevatorSystem));
	// ��¥�������ߵ��������Ϸ���ֱ�ӷ���
	if (numOfFloors <= 0 || numOfFloors > 40 || numOfElevs <= 0 || numOfElevs > 6)
		return R_INVALIDARGS;

	pElevSys->numOfFloors = numOfFloors;
	pElevSys->numOfElevators = numOfElevs;
	int i;
	for (i = 0; i < numOfElevs; ++i)
	{
		pElevSys->Elevators[i].prop = pArrElevPropDesc[i];
		pElevSys->Elevators[i].status = pElevSys->Elevators[i].prevStatus = ELEV_IDLE;
		pElevSys->Elevators[i].pElevSys = pElevSys;
		pElevSys->Elevators[i].currFloor = 1;
	}
		
	return R_OK;
}

RESULT ElevatorSystemCall(_InOut lpElevatorSystem pElevSys, _In int floor, _In BOOL isUp)
{
	if (!pElevSys)
		return R_NULLPTR;

	if (floor < 0 || floor > pElevSys->numOfFloors)
		return R_INVALIDARGS;
	

	// ����Ƿ��нϽ��Ŀ��е��ݿ���ʹ��
	int i, minDiff = INT_MAX, pos = -1;
	Elevator* elev = pElevSys->Elevators;
	for (i = 0; i < pElevSys->numOfElevators; ++i)
	{
		if (elev[i].status == ELEV_IDLE)
		{
			// ����ÿ��е������þ��ڸò㣬ֱ�ӷ���OK
			if (elev[i].currFloor == floor)
				break;
			// Ѱ��¥���ֵ��С������
			int absDiff = abs(elev[i].currFloor - floor);
			minDiff = minDiff < absDiff ? minDiff : absDiff;
			pos = i;
		}
	}

	if (pos != -1)
	{
		elev[pos].status = elev[pos].currFloor > floor ? ELEV_GOINGDOWN : ELEV_GOINGUP;
		elev[pos].prevStatus = ELEV_IDLE;
		elev[pos].timer = 0;
		return R_OK;
	}
	else
		return R_NOTFOUND;
}

