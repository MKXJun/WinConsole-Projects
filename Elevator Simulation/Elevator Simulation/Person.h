#ifndef _PERSON_H_
#define _PERSON_H_
#include "Common.h"
#include "List.h"

typedef struct Person
{
	int inFloor;			// 起始(或居住)楼层
	int outFloor;			// 目标楼层
	int giveUpTime;			// 容忍等候时间
	int currWaitingTime;	// 当前等候时间
	int startTime;			// 出现时间
} Person;





#endif