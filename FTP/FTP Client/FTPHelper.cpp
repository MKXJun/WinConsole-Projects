#include "FTPHelper.h"

const char * GetFTPStatusString(FTPStatusCode code)
{
	switch (code)
	{
	case FTPStat_UserConnected: return "[220]Confirm:已连接到服务器!";
	case FTPStat_UserLoggedIn: return "[230]Confirm:登陆成功!";
	case FTPStat_FileOperationComfirm: return "[250]Confirm:已确认文件操作!";
	case FTPStat_ArgumentSyntaxError: return "[501]Error:语法错误!";
	case FTPStat_InvalidUserNameOrPassWord: return "[530]Error:用户名或密码错误!/当前未登录!";
	case FTPStat_NoSuchDirectoryOrFile: return "[550]Error:该路径或文件不存在!";
	default: return "";
	}
}
