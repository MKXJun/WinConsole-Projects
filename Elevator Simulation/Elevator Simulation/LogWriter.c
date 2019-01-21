#include "LogWriter.h"

static LogWriter logWriter;



RESULT LogWrite(_In const char * str)
{
	if (!logWriter.fp)
	{
		fopen_s(&logWriter.fp, "log.txt", "w");
		// 若仍打不开，则返回失败
		if (!logWriter.fp)
			return R_ERROR;
	}
	logWriter.currIndex = (logWriter.currIndex + 1) % 8;
	memcpy_s(logWriter.strs[logWriter.currIndex], 60, str, strlen(str) + 1);
	fputs(str, logWriter.fp);
	fputc('\n', logWriter.fp);

	return R_OK;
}
