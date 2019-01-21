#ifndef _ELEVATOR_H_
#define _ELEVATOR_H_
#include "Common.h"
#include "List.h"
#include "Person.h"

typedef int ELEV_STATUS;
typedef struct ElevatorSystem ElevatorSystem, *lpElevatorSystem;
// ����״̬
#define ELEV_OFF		0			// ���ݹر�
#define ELEV_IDLE		0x1			// ��������
#define ELEV_OPENING	0x2			// ���ݿ�����
#define ELEV_CLOSING	0x4			// ���ݹ�����
#define ELEV_WAITING	0x8			// ���ݵȺ���
#define ELEV_GOINGUP	0x10		// ����������
#define ELEV_GOINGDOWN	0x20		// �����½���

// ������������
typedef struct
{
	int maxLoad;		// ��������غ�
	int	openingTime;	// ���ݿ�����ʱ
	int	closingTime;	// ���ݹ�����ʱ
	int	waitingTime;	// ���ݵȺ���ʱ
	int	staticTime;		// ���ݾ�ֹʱ��
	int	upTime;			// ��������һ����ʱ
	int	downTime;		// �����½�һ����ʱ
} ElevatorPropDesc;

// ����
typedef struct Elevator
{
	ElevatorPropDesc	prop;				// ��ǰ��������
	int					currFloor;			// ��ǰ¥��
	int					currLoad;			// ��ǰ�غ�
	ELEV_STATUS			status;				// ����״̬
	ELEV_STATUS			prevStatus;			// ��¼����֮ǰ����¥��������¥
	int					timer;				// ��ʱ��

	int					entryTimer;			// ���˽���ʱ�õ��ļ�ʱ��
	List				people;				// ��Ⱥ

	BOOL				buttonFloor[41];	// �����ڲ���ť״̬
	lpElevatorSystem	pElevSys;			// ������������ϵͳ

} Elevator, *lpElevator;


#endif