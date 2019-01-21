#ifndef _COMMON_H_
#define _COMMON_H_

#define _In								// 输入形参
#define _Out							// 输出形参
#define _InOut							// 输入兼输出形参

typedef int RESULT;
#define R_NOTFOUND		-1	// 未找到
#define R_OK			0	// 正常
#define R_ERROR			1	// 错误
#define R_NULLPTR		2	// 参数空指针错误
#define R_BADALLOC		3	// 分配内存异常
#define R_INVALIDARGS	4	// 不合法参数

typedef int BOOL;
#define TRUE		1
#define FALSE		0
#endif