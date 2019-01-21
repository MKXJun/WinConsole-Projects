#ifndef _LOGWRITER_H_
#define _LOGWRITER_H_
#include <stdio.h>
#include "Common.h"
#include <string.h>
// 日志输出者
typedef struct LogWriter
{
	FILE *fp;			// 文件指针
	char strs[8][60];	// 临时保存最近8行字符串
	int lastIndex;		// 指向字符串数组最新输出字符串的索引
	int currIndex;		// 当前索引
} LogWriter;

RESULT LogWrite(_In const char * str);


#endif