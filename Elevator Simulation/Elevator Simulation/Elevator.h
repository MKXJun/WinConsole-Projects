#ifndef _ELEVATOR_H_
#define _ELEVATOR_H_
#include "Common.h"
#include "List.h"
#include "Person.h"

typedef int ELEV_STATUS;
typedef struct ElevatorSystem ElevatorSystem, *lpElevatorSystem;
// 电梯状态
#define ELEV_OFF		0			// 电梯关闭
#define ELEV_IDLE		0x1			// 电梯闲置
#define ELEV_OPENING	0x2			// 电梯开门中
#define ELEV_CLOSING	0x4			// 电梯关门中
#define ELEV_WAITING	0x8			// 电梯等候中
#define ELEV_GOINGUP	0x10		// 电梯上升中
#define ELEV_GOINGDOWN	0x20		// 电梯下降中

// 电梯属性描述
typedef struct
{
	int maxLoad;		// 电梯最大载荷
	int	openingTime;	// 电梯开门用时
	int	closingTime;	// 电梯关门用时
	int	waitingTime;	// 电梯等候用时
	int	staticTime;		// 电梯静止时间
	int	upTime;			// 电梯上升一层用时
	int	downTime;		// 电梯下降一层用时
} ElevatorPropDesc;

// 电梯
typedef struct Elevator
{
	ElevatorPropDesc	prop;				// 当前电梯属性
	int					currFloor;			// 当前楼层
	int					currLoad;			// 当前载荷
	ELEV_STATUS			status;				// 电梯状态
	ELEV_STATUS			prevStatus;			// 记录电梯之前是上楼还是在下楼
	int					timer;				// 计时器

	int					entryTimer;			// 有人进入时用到的计时器
	List				people;				// 人群

	BOOL				buttonFloor[41];	// 电梯内部按钮状态
	lpElevatorSystem	pElevSys;			// 电梯所属管理系统

} Elevator, *lpElevator;


#endif