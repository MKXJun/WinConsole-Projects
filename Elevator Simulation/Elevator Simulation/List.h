#ifndef _LIST_H_
#define _LIST_H_
#include "Common.h"
#include <string.h>
#include <stdlib.h>
// �������ݽṹ��
typedef struct Data
{
	void *pData;	// ���ݶ�
	size_t size;	// �ֽڴ�С
} Data;

// ����ڵ�
typedef struct Node
{
	struct Node *prev;
	struct Node *next;
	Data data;
} Node;

// ����
typedef struct List
{
	Node *front;
	Node *back;
	size_t length;
} List;

// ��ʼ��˫������
RESULT ListInit(_InOut List *pList);
// ��ӽڵ�������ͷ
RESULT ListPushFront(_InOut List *pList, _In Data data);
// ��ӽڵ�������β
RESULT ListPushBack(_InOut List *pList, _In Data data);
// ɾ������ͷ�ڵ�
RESULT ListPopFront(_InOut List *pList);
// ɾ������β�ڵ�
RESULT ListPopBack(_InOut List *pList);
// �������Ƴ���һ���ڵ�
RESULT ListRemoveFirst(_InOut List *pList, _In Data data);
// �������Ƴ����һ���ڵ�
RESULT ListRemoveLast(_InOut List *pList, _In Data data);
// ���ض������Ƴ��������������Ľڵ�
RESULT ListRemove_If(_InOut List *pList, _In BOOL(*pFunc)(_In Data data));
// �������
RESULT ListClear(_InOut List *pList);
// ��ȡ����ͷ
RESULT ListFront(_In List *pList, _Out Data *pOut);
// ��ȡ����β
RESULT ListBack(_In List *pList, _Out Data *pOut);
// ���ض�������ѯ����ڵ�����
RESULT ListFind_If(_In List *pList, _In BOOL(*pFunc)(_In Data data), _Out Data *pOut);
// ���������нڵ���б���
RESULT ListForeach(_In List *pList, _In void(*pFunc)(_In Data data));


#endif