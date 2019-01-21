#ifndef _ELEVATORSYSTEM_H_
#define _ELEVATORSYSTEM_H_
#include "Elevator.h"

typedef struct Simulator Simulator;
// ���ݹ���ϵͳ
typedef struct ElevatorSystem
{
	int			numOfElevators;		// ������Ŀ���������6���� 
	Elevator	Elevators[6];		// ��Ͻ�����е���

	int			numOfFloors;		// ���ݲ������������40��+���²㣩
	BOOL		buttonUp[41];		// �����ⲿ��¥��ť״̬
	BOOL		buttonDown[41];		// �����ⲿ��¥��ť״̬
	Simulator*  pSim;				// ����ģ����
} ElevatorSystem, *lpElevatorSystem;


#endif