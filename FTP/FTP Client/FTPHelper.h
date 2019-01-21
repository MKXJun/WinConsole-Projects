#pragma once

#define FTP_STATUS_ERR(Code) (#Code)


enum FTPStatusCode
{

	FTPStat_UserConnected = 220,
	FTPStat_UserClosed = 221,
	FTPStat_UserLoggedIn = 230,
	FTPStat_FileOperationComfirm = 250,
	FTPStat_NeedPassword = 331,
	FTPStat_NoPermission = 450,
	FTPStat_ArgumentSyntaxError = 501,
	FTPStat_InvalidUserNameOrPassWord = 530,
	FTPStat_UserNotLogIn = 530,
	FTPStat_NoSuchDirectoryOrFile = 550
};

const char * GetFTPStatusString(FTPStatusCode code);
