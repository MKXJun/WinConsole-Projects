#pragma once


#include <thread>
#include "FTP.h"
#include "Console.h"
#include <queue>

#pragma comment(lib, "WS2_32")

class FTPClient : public IClient
{
public:
	FTPClient();

	void Run();

private:
	void OpenCtrl(std::istringstream& iss);
	void CloseCtrl(std::istringstream& iss);
	
	void BeginTransfer(const std::string& cmd);

	void ShowCurrPath();
	void ShowHelp(std::istringstream& iss);

private:
	void Parse();
	bool ParseRest(std::istringstream& iss);
	std::string PathAppend(std::string curr, std::string extend);
	void ConsoleInput();
	void RollLine();

	FTPStatusCode GetAndWriteFTPStatusCode();

	void InitWindow();

private:
	SOCKET msCtrl;		// 控制部分
	SOCKET msData;		// 数据传输部分
	sockaddr_in mServerAddr;
	FTPStatusCode mFTPCtrlStatusCode, mFTPDataStatusCode;
	Console& mConsole;
	std::string mCurrPath;
	std::vector<std::string> mDirectories;
	std::thread mtData;
	std::vector<char> mCtrlBuffer, mRecvBuffer;
};
