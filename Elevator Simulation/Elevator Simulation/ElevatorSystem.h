#ifndef _ELEVATORSYSTEM_H_
#define _ELEVATORSYSTEM_H_
#include "Elevator.h"

typedef struct Simulator Simulator;
// 电梯管理系统
typedef struct ElevatorSystem
{
	int			numOfElevators;		// 电梯数目（最多允许6部） 
	Elevator	Elevators[6];		// 管辖的所有电梯

	int			numOfFloors;		// 电梯层数（最高允许40层+地下层）
	BOOL		buttonUp[41];		// 电梯外部上楼按钮状态
	BOOL		buttonDown[41];		// 电梯外部下楼按钮状态
	Simulator*  pSim;				// 所属模拟器
} ElevatorSystem, *lpElevatorSystem;


#endif