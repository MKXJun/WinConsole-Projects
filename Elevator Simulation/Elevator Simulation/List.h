#ifndef _LIST_H_
#define _LIST_H_
#include "Common.h"
#include <string.h>
#include <stdlib.h>
// 泛型数据结构体
typedef struct Data
{
	void *pData;	// 数据段
	size_t size;	// 字节大小
} Data;

// 链表节点
typedef struct Node
{
	struct Node *prev;
	struct Node *next;
	Data data;
} Node;

// 链表
typedef struct List
{
	Node *front;
	Node *back;
	size_t length;
} List;

// 初始化双向链表
RESULT ListInit(_InOut List *pList);
// 添加节点入链表头
RESULT ListPushFront(_InOut List *pList, _In Data data);
// 添加节点入链表尾
RESULT ListPushBack(_InOut List *pList, _In Data data);
// 删除链表头节点
RESULT ListPopFront(_InOut List *pList);
// 删除链表尾节点
RESULT ListPopBack(_InOut List *pList);
// 按数据移除第一个节点
RESULT ListRemoveFirst(_InOut List *pList, _In Data data);
// 按数据移除最后一个节点
RESULT ListRemoveLast(_InOut List *pList, _In Data data);
// 以特定条件移除所有满足条件的节点
RESULT ListRemove_If(_InOut List *pList, _In BOOL(*pFunc)(_In Data data));
// 清空链表
RESULT ListClear(_InOut List *pList);
// 获取链表头
RESULT ListFront(_In List *pList, _Out Data *pOut);
// 获取链表尾
RESULT ListBack(_In List *pList, _Out Data *pOut);
// 以特定条件查询链表节点数据
RESULT ListFind_If(_In List *pList, _In BOOL(*pFunc)(_In Data data), _Out Data *pOut);
// 对链表所有节点进行遍历
RESULT ListForeach(_In List *pList, _In void(*pFunc)(_In Data data));


#endif