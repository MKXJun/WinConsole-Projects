#include "Person.h"
#include "Elevator.h"

void PeopleUpdate(_In Data data)
{
	((Person*)data.pData)->currWaitingTime++;
}

BOOL PeopleTimeOut(_In Data data)
{
	return ((Person*)data.pData)->currWaitingTime >= ((Person*)data.pData)->giveUpTime;
}

BOOL PeopleGoingUp(_In Data data)
{
	return ((Person*)data.pData)->outFloor > ((Person*)data.pData)->inFloor;
}

BOOL PeopleGoingDown(_In Data data)
{
	return ((Person*)data.pData)->outFloor < ((Person*)data.pData)->inFloor;
}