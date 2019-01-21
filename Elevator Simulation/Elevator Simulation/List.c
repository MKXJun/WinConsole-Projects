#include "List.h"

RESULT ListInit(_InOut List * pList)
{
	if (!pList)
		return R_NULLPTR;

	//ȫ������
	memset(pList, 0, sizeof(List));
	return R_OK;
}

RESULT ListPushFront(_InOut List * pList, _In Data data)
{
	if (!pList)
		return R_NULLPTR;

	// �½��ڵ�
	Node *pNode = (Node *)malloc(sizeof(Node));
	memset(pNode, 0, sizeof(Node));
	// ������ȿ���
	pNode->data.pData = malloc(data.size);
	memcpy(pNode->data.pData, data.pData, data.size);
	pNode->data.size = data.size;
	// ��������Ϊ0������ͷβָ��ָ��ýڵ�
	if (pList->length == 0)
		pList->front = pList->back = pNode;
	// ��֮�øýڵ��Ϊ�µ�front
	else
	{
		pNode->next = pList->front;
		pList->front->prev = pNode;
		pNode->prev = NULL;
		pList->front = pNode;
	}
	pList->length++;
	return R_OK;
}

RESULT ListPushBack(_InOut List * pList, _In Data data)
{
	if (!pList)
		return R_NULLPTR;
	Node *pNode = (Node *)malloc(sizeof(Node));
	memset(pNode, 0, sizeof(Node));
	// �������
	pNode->data.pData = malloc(data.size);
	memcpy(pNode->data.pData, data.pData, data.size);
	pNode->data.size = data.size;
	// ��������Ϊ0������ͷβָ��ָ��ýڵ�
	if (pList->length == 0)
		pList->front = pList->back = pNode;
	// ��֮�øýڵ��Ϊ�µ�back
	else
	{
		pNode->prev = pList->back;
		pList->back->next = pNode;
		pNode->next = NULL;
		pList->back = pNode;
	}
	pList->length++;
	return R_OK;
}

RESULT ListPopFront(_InOut List * pList)
{
	if (!pList || pList->length == 0)
		return R_NULLPTR;

	// frontָ����һ�ڵ㣬�ÿոýڵ��prev
	Node *pNode = pList->front;
	pList->front = pList->front->next;
	
	if (pList->front)
		pList->front->prev = NULL;

	free(pNode->data.pData);
	free(pNode);
	pList->length--;
	if (pList->length == 0)
		pList->back = NULL;
	return R_OK;
}

RESULT ListPopBack(_InOut List * pList)
{
	if (!pList || pList->length == 0)
		return R_NULLPTR;
	
	// backָ����һ�ڵ㣬�ÿոýڵ��next
	Node *pNode = pList->back;
	pList->back = pList->back->prev;
	
	if (pList->back)
		pList->back->next = NULL;

	free(pNode->data.pData);
	free(pNode);
	pList->length--;
	if (pList->length == 0)
		pList->front = NULL;
	return R_OK;
}

RESULT ListRemoveFirst(_InOut List * pList, _In Data data)
{
	if (!pList || pList->length == 0)
		return R_NULLPTR;

	Node *pNode = pList->front;
	// ��ͷ��βѰ�ҵ�һ��������ݵĽڵ�
	while (pNode != NULL)
	{
		if (!memcmp(pNode->data.pData, data.pData, data.size)
			&& data.size == pNode->data.size)
			break;
		pNode = pNode->next;
	}
		
	if (pNode == NULL)
		return R_NOTFOUND;

	
	if (pNode->prev != NULL)
		pNode->prev->next = pNode->next;
	else
		pList->front = pNode->next;

	if (pNode->next != NULL)
		pNode->next->prev = pNode->prev;
	else
		pList->back = pNode->prev;

	free(pNode->data.pData);
	free(pNode);
	pList->length--;
	if (pList->length == 0)
		pList->back = NULL;
	return R_OK;
}

RESULT ListRemoveLast(_InOut List * pList, _In Data data)
{
	if (!pList || pList->length == 0)
		return R_NULLPTR;

	Node *pNode = pList->back;
	// ��β��ͷѰ�ҵ�һ��������ݵĽڵ�
	while (pNode != NULL)
	{
		if (!memcmp(pNode->data.pData, data.pData, data.size)
			&& data.size == pNode->data.size)
			break;
		pNode = pNode->prev;
	}
	if (pNode == NULL)
		return R_NOTFOUND;
	
	if (pNode->prev != NULL)
		pNode->prev->next = pNode->next;
	else
		pList->front = pNode->next;

	if (pNode->next != NULL)
		pNode->next->prev = pNode->prev;
	else
		pList->back = pNode->prev;

	free(pNode->data.pData);
	free(pNode);
	pList->length--;
	if (pList->length == 0)
		pList->front = NULL;
	return R_OK;
}

RESULT ListRemove_If(_InOut List * pList, _In BOOL(*pFunc)(_In Data data))
{
	if (!pList || !pFunc || pList->length == 0)
		return R_NULLPTR;

	Node *pNode = pList->front;
	// ��ͷ��βѰ���������������Ľڵ�
	while (pNode != NULL)
	{
		if (pFunc(pNode->data) == TRUE)
		{
			Node *pNext = pNode->next;
			if (pNode->prev != NULL)
				pNode->prev->next = pNext;
			else
				pList->front = pNext;

			if (pNext != NULL)
				pNext->prev = pNode->prev;
			else
				pList->back = pNode->prev;

			free(pNode->data.pData);
			free(pNode);
			pList->length--;
			pNode = pNext;
		}
		else
			pNode = pNode->next;
	}
	if (pList->length == 0)
		pList->back = NULL;
	return R_OK;
}

RESULT ListClear(_InOut List * pList)
{
	if (!pList)
		return R_NULLPTR;

	while (pList->length > 0)
		ListPopFront(pList);
	return R_OK;
}

RESULT ListFront(_In List * pList, _Out Data * pOut)
{
	if (!pOut)
		return R_NULLPTR;
	if (!pList || !pList->front)
	{
		memset(pOut, 0, sizeof(Data));
		return R_NULLPTR;
	}
	memcpy(pOut, &pList->front->data, sizeof(Data));
	return R_OK;
}

RESULT ListBack(_In List * pList, _Out Data * pOut)
{
	if (!pOut)
		return R_NULLPTR;
	if (!pList || !pList->back)
	{
		memset(pOut, 0, sizeof(Data));
		return R_NULLPTR;
	}
	memcpy(pOut, &pList->back->data, sizeof(Data));
	return R_OK;
}


RESULT ListFind_If(_In List * pList, _In BOOL(*pFunc)(_In Data data), _Out Data * pOut)
{
	if (!pOut || !pFunc)
		return R_NULLPTR;
	if (!pList)
	{
		memset(pOut, 0, sizeof(Data));
		return R_NULLPTR;
	}

	Node *pNode = pList->front;
	while (pNode)
	{
		if (pFunc(pNode->data) == TRUE)
		{
			memcpy(pOut, &pNode->data, sizeof(Data));
			return R_OK;
		}
		pNode = pNode->next;
	}
	memset(pOut, 0, sizeof(Data));
	return R_NOTFOUND;
}

RESULT ListForeach(_In List * pList, _In void(*pFunc)(_In Data data))
{
	if (!pList || !pFunc)
		return R_NULLPTR;
	Node* pNode = pList->front;
	while (pNode)
	{
		pFunc(pNode->data);
		pNode = pNode->next;
	}
	return R_OK;
}

