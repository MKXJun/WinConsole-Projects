#include "Elevator.h"
#include "Simulator.h"
void ElevatorIdle(_InOut lpElevator pElev)
{
	// 若停靠楼层的外部按钮被按下，转为开门
	if (pElev->pElevSys->buttonUp[pElev->currFloor] == TRUE)
	{
		pElev->status = ELEV_OPENING;
		pElev->prevStatus = ELEV_GOINGUP;
		pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
		pElev->timer = 0;

		// 日志输出
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		return;
	}
	else if (pElev->pElevSys->buttonDown[pElev->currFloor] == TRUE)
	{
		pElev->status = ELEV_OPENING;
		pElev->prevStatus = ELEV_GOINGDOWN;
		pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
		pElev->timer = 0;

		// 日志输出
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		return;
	}


	// 电梯停在一楼以外的地方，并且静止时间到达staticTime时，将会自动
	// 回到一楼
	if (pElev->timer >= pElev->prop.staticTime && pElev->currFloor != 1)
	{
		pElev->buttonFloor[1] = TRUE;
		// 在地下层就让它上升
		if (pElev->currFloor == 0)
			pElev->prevStatus = pElev->status = ELEV_GOINGUP;
		else
			pElev->prevStatus = pElev->status = ELEV_GOINGDOWN;
		pElev->timer = 0;

		// 日志输出
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯将回到1楼\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
	}
}

void ElevatorOpening(_InOut lpElevator pElev)
{
	// 电梯开门时间到达openingTime时，转为等候状态
	if (pElev->timer >= pElev->prop.openingTime)
	{
		pElev->status = ELEV_WAITING;
		pElev->timer = 0;

		// 日志输出
		fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯等候中\n",
			pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
	}
}

void ElevatorWaiting(_InOut lpElevator pElev)
{
	// 电梯等待时间到达waitingTime
	if (pElev->timer >= pElev->prop.waitingTime)
	{

		// 没有人进入时将转为关门状态
		if (pElev->entryTimer == 0)
		{
			pElev->status = ELEV_CLOSING;
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯关门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		else
		{
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯检测到有人进出\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}

		pElev->timer = 0;
	}
}

void ElevatorClosing(_InOut lpElevator pElev)
{
	// 电梯在关门前有人想上楼或者下楼，且此时电梯没有超载，则重新开门
	if (pElev->currLoad < pElev->prop.maxLoad)
	{
		if (pElev->prevStatus == ELEV_GOINGUP && pElev->pElevSys->buttonUp[pElev->currFloor])
		{
			pElev->status = ELEV_OPENING;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->timer = 0;
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			return;
		}
		else if (pElev->prevStatus == ELEV_GOINGDOWN && pElev->pElevSys->buttonDown[pElev->currFloor])
		{
			pElev->status = ELEV_OPENING;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->timer = 0;
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			return;
		}
	}
	// 电梯等待时间到达closingTime
	if (pElev->timer >= pElev->prop.closingTime)
	{
		// 检测离电梯最近的目标上楼、下楼层数
		int minUp, maxDown;
		for (minUp = pElev->currFloor + 1; minUp <= pElev->pElevSys->numOfFloors; ++minUp)
			if (pElev->buttonFloor[minUp])
				break;
		for (maxDown = pElev->currFloor - 1; maxDown >= 0; --maxDown)
			if (pElev->buttonFloor[maxDown])
				break;

		// 若此时内部没有人操作，则转为闲置状态
		if (minUp > pElev->pElevSys->numOfFloors && maxDown < 0)
		{
			pElev->prevStatus = pElev->status = ELEV_IDLE;
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯闲置中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// 若电梯之前在上楼
		else if (pElev->prevStatus == ELEV_GOINGUP)
		{
			// 若此时也有上楼的用户，则转换成上楼状态
			if (minUp <= pElev->pElevSys->numOfFloors)
			{
				pElev->status = ELEV_GOINGUP;
				// 日志输出
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯上升中\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
			// 反之转换成下楼状态
			else
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGDOWN;
				// 日志输出
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯下降中\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
		}
		// 若电梯之前在下楼
		else if (pElev->prevStatus == ELEV_GOINGDOWN)
		{
			// 若此时也有下楼的用户，则转换成下楼状态
			if (maxDown >= 0)
			{
				pElev->status = ELEV_GOINGDOWN;
				// 日志输出
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯下降中\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
			// 反之转换成上楼状态
			else
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGUP;
				// 日志输出
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯上升中\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
				
		}
		// 若电梯之前是闲置的，则选择移动距离最近的楼层
		else if (pElev->prevStatus == ELEV_IDLE)
		{
			// 若此时也有上楼的用户，则转换成下楼状态
			if (pElev->currFloor - maxDown <= minUp - pElev->currFloor)
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGDOWN;
				// 日志输出
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯下降中\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
			// 反之转换成下楼状态
			else
			{
				pElev->prevStatus = pElev->status = ELEV_GOINGUP;
				// 日志输出
				fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯上升中\n",
					pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
			}
				
		}
		pElev->timer = 0;
	}
}

void ElevatorGoingUp(_InOut lpElevator pElev)
{
	// 电梯上楼时间到达upTime
	if (pElev->timer >= pElev->prop.upTime)
	{
		pElev->currFloor++;

		// 到达最顶层
		if (pElev->currFloor == pElev->pElevSys->numOfFloors)
		{
			// 清掉内部和外部按钮状态并开门
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->prevStatus = ELEV_GOINGDOWN;
			pElev->status = ELEV_OPENING;

			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// 该层内部按钮或者外部上楼按钮被按下
		else if (pElev->buttonFloor[pElev->currFloor] ||
			pElev->pElevSys->buttonUp[pElev->currFloor])
		{
			// 清掉内部和外部按钮状态并开门
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->prevStatus = ELEV_GOINGUP;
			pElev->status = ELEV_OPENING;

			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		
		// 电梯之前是闲置的，此时外部下楼按钮被按下
		else if ((pElev->prevStatus == ELEV_IDLE && 
			pElev->pElevSys->buttonDown[pElev->currFloor]))
		{
			// 清掉内部和外部按钮状态并开门
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGDOWN;

			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		else
		{
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯上升中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}	
		pElev->timer = 0;
	}
}

void ElevatorGoingDown(_InOut lpElevator pElev)
{
	// 电梯下楼时间到达downTime
	if (pElev->timer >= pElev->prop.downTime)
	{
		pElev->currFloor--;
		// 到达最底层
		if (pElev->currFloor == 0)
		{
			// 清掉内部和外部按钮状态并开门
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGUP;

			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// 该层内部按钮或者外部下楼按钮被按下
		else if (pElev->buttonFloor[pElev->currFloor] ||
			pElev->pElevSys->buttonDown[pElev->currFloor])
		{
			// 清掉内部和外部按钮状态并开门
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonDown[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGDOWN;

			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		// 电梯之前是闲置的，此时外部上楼按钮被按下
		else if ((pElev->prevStatus == ELEV_IDLE &&
			pElev->pElevSys->buttonUp[pElev->currFloor]))
		{
			// 清掉内部和外部按钮状态并开门
			pElev->buttonFloor[pElev->currFloor] = FALSE;
			pElev->pElevSys->buttonUp[pElev->currFloor] = FALSE;
			pElev->status = ELEV_OPENING;
			pElev->prevStatus = ELEV_GOINGUP;

			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯开门中\n",
				pElev->pElevSys->pSim->timer, pElev - pElev->pElevSys->Elevators + 1, pElev->currFloor);
		}
		else
		{
			// 日志输出
			fprintf(pElev->pElevSys->pSim->fpLog, "%-10d电梯%d[%d楼]: 电梯下降中\n",
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

