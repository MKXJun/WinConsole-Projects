#ifndef _LOGWRITER_H_
#define _LOGWRITER_H_
#include <stdio.h>
#include "Common.h"
#include <string.h>
// ��־�����
typedef struct LogWriter
{
	FILE *fp;			// �ļ�ָ��
	char strs[8][60];	// ��ʱ�������8���ַ���
	int lastIndex;		// ָ���ַ���������������ַ���������
	int currIndex;		// ��ǰ����
} LogWriter;

RESULT LogWrite(_In const char * str);


#endif