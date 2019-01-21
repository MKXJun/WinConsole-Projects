#include "Elevator.h"
#include "Simulator.h"
void ElevatorIdle(_InOut lpElevator pElev)
{
	// ��ͣ��¥����ⲿ��ť�����£�תΪ����
	if (pElev->pElevSys->buttonUp[pElev->currFloor] == TRUE)
	{
		pElev->status = ELEV_OPENING;
		pElev->prevStatus = ELEV_GOINGUP;
		pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
		pElev->timer = 0;

		// ��־���
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		return;
	}
	else if (pElev->pElevSys->buttonDown[pElev->currFloor] == TRUE)
	{
		pElev->status = ELEV_OPENING;
		pElev->prevStatus = ELEV_GOINGDOWN;
		pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
		pElev->timer = 0;

		// ��־���
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		return;
	}


	// ����ͣ��һ¥����ĵط������Ҿ�ֹʱ�䵽��staticTimeʱ�������Զ�
	// �ص�һ¥
	if (pElev->timer >= pElev->prop.staticTime && pElev->currFloor != 1)
	{
		pElev->buttonFloor[1] = TRUE;
		// �ڵ��²����������
		if (pElev->currFloor == 0)
			pElev->prevStatus = pElev->status = ELEV_GOINGUP;
		else
			pElev->prevStatus = pElev->status = ELEV_GOINGDOWN;
		pElev->timer = 0;

		// ��־���
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݽ��ص�1¥\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
	}
}

void ElevatorOpening(_InOut lpElevator pElev)
{
	// ���ݿ���ʱ�䵽��openingTimeʱ��תΪ�Ⱥ�״̬
	if (pElev->timer >= pElev->prop.openingTime)
	{
		pElev->status = ELEV_WAITING;
		pElev->timer = 0;

		// ��־���
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݵȺ���\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
	}
}

void ElevatorWaiting(_InOut lpElevator pElev)
{
	// ���ݵȴ�ʱ�䵽��waitingTime
	if (pElev->timer >= pElev->prop.waitingTime)
	{

		// û���˽���ʱ��תΪ����״̬
		if (pElev->entryTimer == 0)
		{
			pElev->status = ELEV_CLOSING;
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݹ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		else
		{
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݼ�⵽���˽���\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}

		pElev->timer = 0;
	}
}

void ElevatorClosing(_InOut lpElevator pElev)
{
	// �����ڹ���ǰ��������¥������¥���Ҵ�ʱ����û�г��أ������¿���
	if (pElev->currLoad < pElev->prop.maxLoad)
	{
		if (pElev->prevStatus == ELEV_GOINGUP && pElev->pElevSys->buttonUp[pElev->currFloor])
		{
			pElev->status = ELEV_OPENING;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->timer = 0;
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			return;
		}
		else if (pElev->prevStatus == ELEV_GOINGDOWN && pElev->pElevSys->buttonDown[pElev->currFloor])
		{
			pElev->status = ELEV_OPENING;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->timer = 0;
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			return;
		}
	}
	// ���ݵȴ�ʱ�䵽��closingTime
	if (pElev->timer >= pElev->prop.closingTime)
	{
		// �������������Ŀ����¥����¥����
		int minUp, maxDown;
		for (minUp = pElev->currFloor + 1; minUp <= pElev->pElevSys->numOfFloors; ++minUp)
			if (pElev->buttonFloor[minUp])
				break;
		for (maxDown = pElev->currFloor - 1; maxDown >= 0; --maxDown)
			if (pElev->buttonFloor[maxDown])
				break;

		// ����ʱ�ڲ�û���˲�������תΪ����״̬
		if (minUp > pElev->pElevSys->numOfFloors && maxDown < 0)
		{
			pElev->prevStatus = pElev->status = ELEV_IDLE;
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ����������\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// ������֮ǰ����¥
		else if (pElev->prevStatus == ELEV_GOINGUP)
		{
			// ����ʱҲ����¥���û�����ת������¥״̬
			if (minUp <= pElev->pElevSys->numOfFloors)
			{
				pElev->status = ELEV_GOINGUP;
				// ��־���
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ����������\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
			// ��֮ת������¥״̬
			else
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGDOWN;
				// ��־���
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: �����½���\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
		}
		// ������֮ǰ����¥
		else if (pElev->prevStatus == ELEV_GOINGDOWN)
		{
			// ����ʱҲ����¥���û�����ת������¥״̬
			if (maxDown >= 0)
			{
				pElev->status = ELEV_GOINGDOWN;
				// ��־���
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: �����½���\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
			// ��֮ת������¥״̬
			else
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGUP;
				// ��־���
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ����������\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
				
		}
		// ������֮ǰ�����õģ���ѡ���ƶ����������¥��
		else if (pElev->prevStatus == ELEV_IDLE)
		{
			// ����ʱҲ����¥���û�����ת������¥״̬
			if (pElev->currFloor - maxDown <= minUp - pElev->currFloor)
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGDOWN;
				// ��־���
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: �����½���\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
			// ��֮ת������¥״̬
			else
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGUP;
				// ��־���
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ����������\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
				
		}
		pElev->timer = 0;
	}
}

void ElevatorGoingUp(_InOut lpElevator pElev)
{
	// ������¥ʱ�䵽��upTime
	if (pElev->timer >= pElev->prop.upTime)
	{
		pElev->currFloor++;

		// �������
		if (pElev->currFloor == pElev->pElevSys->numOfFloors)
		{
			// ����ڲ����ⲿ��ť״̬������
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->prevStatus = ELEV_GOINGDOWN;
			pElev->status = ELEV_OPENING;

			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// �ò��ڲ���ť�����ⲿ��¥��ť������
		else if (pElev->buttonFloor[pElev->currFloor] ||
			pElev->pElevSys->buttonUp[pElev->currFloor])
		{
			// ����ڲ����ⲿ��ť״̬������
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->prevStatus = ELEV_GOINGUP;
			pElev->status = ELEV_OPENING;

			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		
		// ����֮ǰ�����õģ���ʱ�ⲿ��¥��ť������
		else if ((pElev->prevStatus == ELEV_IDLE && 
			pElev->pElevSys->buttonDown[pElev->currFloor]))
		{
			// ����ڲ����ⲿ��ť״̬������
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGDOWN;

			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		else
		{
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ����������\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}	
		pElev->timer = 0;
	}
}

void ElevatorGoingDown(_InOut lpElevator pElev)
{
	// ������¥ʱ�䵽��downTime
	if (pElev->timer >= pElev->prop.downTime)
	{
		pElev->currFloor--;
		// ������ײ�
		if (pElev->currFloor == 0)
		{
			// ����ڲ����ⲿ��ť״̬������
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGUP;

			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// �ò��ڲ���ť�����ⲿ��¥��ť������
		else if (pElev->buttonFloor[pElev->currFloor] ||
			pElev->pElevSys->buttonDown[pElev->currFloor])
		{
			// ����ڲ����ⲿ��ť״̬������
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGDOWN;

			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// ����֮ǰ�����õģ���ʱ�ⲿ��¥��ť������
		else if ((pElev->prevStatus == ELEV_IDLE &&
			pElev->pElevSys->buttonUp[pElev->currFloor]))
		{
			// ����ڲ����ⲿ��ť״̬������
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGUP;

			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: ���ݿ�����\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		else
		{
			// ��־���
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d����%d[%d¥]: �����½���\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		pElev->timer = 0;
	}
}

void ElevatorUpdate(_InOut lpElevator pElev)
{
	if (!pElev || pElev->status == ELEV_OFF)
		return;

	++pElev->timer;

	switch (pElev->status)
	{
	case ELEV_IDLE: ElevatorIdle(pElev); break;
	case ELEV_OPENING: ElevatorOpening(pElev); break;
	case ELEV_WAITING: ElevatorWaiting(pElev); break;
	case ELEV_CLOSING: ElevatorClosing(pElev); break;
	case ELEV_GOINGUP: ElevatorGoingUp(pElev); break;
	case ELEV_GOINGDOWN: ElevatorGoingDown(pElev); break;
	}

}

