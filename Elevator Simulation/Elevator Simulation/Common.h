#ifndef _COMMON_H_
#define _COMMON_H_

#define _In								// �����β�
#define _Out							// ����β�
#define _InOut							// ���������β�

typedef int RESULT;
#define R_NOTFOUND		-1	// δ�ҵ�
#define R_OK			0	// ����
#define R_ERROR			1	// ����
#define R_NULLPTR		2	// ������ָ�����
#define R_BADALLOC		3	// �����ڴ��쳣
#define R_INVALIDARGS	4	// ���Ϸ�����

typedef int BOOL;
#define TRUE		1
#define FALSE		0
#endif