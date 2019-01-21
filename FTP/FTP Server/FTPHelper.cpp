#include "FTPHelper.h"

const char * GetFTPStatusString(FTPStatusCode code)
{
	switch (code)
	{
	case FTPStat_UserConnected: return "[220]Confirm:�����ӵ�������!";
	case FTPStat_UserLoggedIn: return "[230]Confirm:��½�ɹ�!";
	case FTPStat_FileOperationComfirm: return "[250]Confirm:��ȷ���ļ�����!";
	case FTPStat_ArgumentSyntaxError: return "[501]Error:�﷨����!";
	case FTPStat_InvalidUserNameOrPassWord: return "[530]Error:�û������������!/��ǰδ��¼!";
	case FTPStat_NoSuchDirectoryOrFile: return "[550]Error:��·�����ļ�������!";
	default: return "";
	}
}
