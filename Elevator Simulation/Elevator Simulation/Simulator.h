#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include <stdio.h>
#include "ElevatorSystem.h"

// 模拟器
typedef struct Simulator
{
	ElevatorSystem  elevSys;
	List            people[41];	// 最高允许40层楼
	int			    timer;		// 计时器
	int				endTime;	// 结束时间
	int				delayTime;	// 帧延迟时间
	BOOL			isEnd;		// 模拟是否结束


	int				peopleMinWaitingTime;	// 人最小容忍等待时间
	int				peopleMaxWaitingTime;	// 人最大容忍等待时间
	int				peopleLeft[41];			// 剩余人数
	int				peopleTimeOut[41];		// 超时人数
	int				peopleStartCount[41];	// 初始人数

	FILE*			fpLog;					// 日志文件指针
} Simulator;

// 采用输入的形式来初始化模拟器
RESULT SimInputInit(_InOut Simulator* pSim);

// 读取sim.txt数据来初始化模拟器
RESULT SimLoadInit(_InOut Simulator* pSim);

// 保存输入的数据到文件
RESULT SimSave(_In const Simulator* pSim);

// 更新函数
RESULT SimUpdate(_InOut Simulator* pSim);

// 人员随机生成器
RESULT SimPeopleRandomGenerator(_InOut Simulator* pSim);

// 实时显示电梯模拟数据
RESULT SimRealTimeElevShow(_In HANDLE hOut, _In Simulator* pSim);

// 关闭模拟器
RESULT SimClose(_InOut Simulator* pSim);





#endif
