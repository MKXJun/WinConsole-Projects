#pragma once


#include <thread>
#include <queue>
#include <fstream>
#include "FTP.h"
#include <filesystem>
#include "Console.h"

#pragma comment(lib, "WS2_32")





struct VoidDeleter
{
	void operator()(void * block) { free(block); }
};






class FTPServer : public IServer
{
public:
	typedef std::experimental::filesystem::path FilePath;

	struct ClientInfo
	{
		sockaddr_in ipAddr;		// 客户端IP地址
		SOCKET socket;			// 客户端套接字
		bool loggined;			// 客户端已经登录
	};

	struct ClientEvent
	{
		uint32_t clientIdx;			// 当前事件发生时客户端索引
		char userEvent[8];			// 事件类型
		FTPStatusCode statusCode;	// 状态码
		char str1[MAX_PATH];		// 语法拆分的第一个字符串
		char str2[MAX_PATH];		// 语法拆分的第二个字符串
	};
public:
	FTPServer();
	virtual ~FTPServer();

	// IServer接口实现
	void Run() override;
	
private:
	bool LogIn(char *userName, char *passWord);
	bool SetCurrDir(const FilePath& path);
	bool SetUpDir();
	bool Rename(const FilePath& source, const FilePath& target);
	bool MakeDir(const FilePath& path);
	bool GetFile(const FilePath& fromFile);
	bool Delete(const FilePath& p);

private:
	// 语法分析部分
	void Parse(char * buffer, uint32_t bufferCount);
	bool ParsePath(std::istringstream& iss, int EventStr);
	bool ParseRest(std::istringstream& iss);
	std::string PathAppend(std::string curr, std::string extend);
	// 通用工具
	void ShowHostIP();
	char* ShowServerTime(char * outBuffer, uint32_t bufferCount);
	void WriteLog();
	void SendCurrDirectories();
	void SendFile(SOCKET s, std::string file);
	void RecvFile(SOCKET s, std::string file);
	// 主循环部分
	void UserCtrlConnect();
	void UserDataConnect();
	void UserSendCtrlMessage();
	void UserSendDataMessage();
	
	

private:
	// 控制部分
	SOCKET								msCtrl;				// 服务器控制监听套接字
	WSAEVENT							mhCtrl;				// WSA服务器控制事件
	std::vector<WSAEVENT>				mhClientCtrls;		// 客户端控制事件
	std::vector<ClientInfo>				mClientCtrlInfos;	// 客户端控制信息
	
	
	

	// 传输部分
	SOCKET								msData;				// 服务器数据监听套接字
	WSAEVENT							mhData;				// WSA服务器数据事件
	std::vector<WSAEVENT>				mhClientDatas;		// 客户端数据事件
	std::vector<ClientInfo>				mClientDataInfos;	// 客户端数据信息
	std::vector<std::thread>			mDataThreads;		// 数据进程

	std::ofstream						mLog;				// 日志文件流

	std::vector<std::unique_ptr<void, VoidDeleter>>	mUserInfos;	// 客户信息
	uint32_t							mUserBlockSize;			// 单个用户块占用内存大小

	ClientEvent							mClientEvent;		// 当前客户端事件

	uint16_t							mPortControl;		// 控制端口
	uint16_t							mPortData;			// 数据端口


	std::experimental::filesystem::path mRootPath;			// 可访问根目录
	std::experimental::filesystem::path mCurrPath;			// 当前目录

};

bool FTPCreateServer(IServer** pOut);

