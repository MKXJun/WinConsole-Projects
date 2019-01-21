#include "FTPServer.h"
#include <sstream>

using namespace std::experimental::filesystem;

static FTPServer* gServer = nullptr;

FTPServer::FTPServer()
	: msCtrl{},
	msData{},
	mhCtrl{},
	mhData{},
	mhClientCtrls{},
	mhClientDatas{},
	mClientCtrlInfos{},
	mClientDataInfos{},
	mUserInfos{},
	mPortControl{ 21 },
	mPortData{ 20 }
{
	msCtrl = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	msData = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (msCtrl == INVALID_SOCKET || msData == INVALID_SOCKET)
	{
		MessageBoxA(NULL, "SOCKET创建失败", "错误", MB_OK);
		system("PAUSE");
		exit(EXIT_FAILURE);
	}
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_port = htons(mPortControl);
	if (::bind(msCtrl, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		MessageBox(NULL, "绑定失败", "错误", MB_OK);
		exit(EXIT_FAILURE);
	}
	sin.sin_port = htons(mPortData);
	if (::bind(msData, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		MessageBox(NULL, "绑定失败", "错误", MB_OK);
		exit(EXIT_FAILURE);
	}

	::listen(msCtrl, 200);
	::listen(msData, 200);

	// 创建事件对象，并关联到新的套节字
	mhCtrl = ::WSACreateEvent();
	::WSAEventSelect(msCtrl, mhCtrl, FD_ACCEPT | FD_CLOSE);
	mhData = ::WSACreateEvent();
	::WSAEventSelect(msData, mhData, FD_ACCEPT | FD_CLOSE);


	// 获取当前用于存储的路径
	mCurrPath = current_path();
	mRootPath = mCurrPath += "\\SaveData\\";

	// 检验路径是否存在，若否，则创建该路径
	if (!exists(mRootPath))
	{
		create_directory(mRootPath);
	}

	// 发送服务器开启成功通知
	Console::WriteLine("服务器开启成功!");

	// 开启日志
	mLog.open("Log.txt", std::ios::out | std::ios::app);
}


void FTPServer::Run()
{
	ShowHostIP();

	// 主循环
	while (TRUE)
	{

		// 检测服务端控制套接字是否有响应
		UserCtrlConnect();
		// 检测服务端数据套接字是否有响应
		UserDataConnect();

		// 检测用户是否有发送控制消息，进行处理并回发状态码
		UserSendCtrlMessage();
		// 检测用户是否有发送数据消息，进行处理
		UserSendDataMessage();

	}
}



bool FTPServer::LogIn(char * userName, char * passWord)
{
	// TODO
	if (!exists("user.dat"))
	{
		std::ofstream fout("user.dat", std::ios::binary | std::ios::out);
		fout.close();
	}
	std::ifstream fin("user.dat", std::ios::binary | std::ios::in);
	char un[21]{}, pw[21]{};
	fin.seekg(0);
	while (!fin.eof())
	{
		fin.read(un, 20).read(pw, 20);
		if (strcmp(un, userName) == 0)
		{
			if (strcmp(pw, passWord))
				break;
			else
			{
				fin.close();
				return true;
			}
		}
	}
	fin.close();
	return false;
}

bool FTPServer::SetCurrDir(const FilePath & path)
{
	std::string cwp = PathAppend(mCurrPath.string(), path.string());
	// 不存在路径或者路径缩的过短的情况
	if (!exists(cwp) || mRootPath.string().length() > cwp.length())
	{
		mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
		return false;
	}
		
	mClientEvent.statusCode = FTPStat_FileOperationComfirm;
	mCurrPath = cwp;
	return true;
}

bool FTPServer::SetUpDir()
{
	return SetCurrDir("..\\");
}

bool FTPServer::Rename(const FilePath & source, const FilePath & target)
{
	std::string src = PathAppend(mCurrPath.string(), source.string());
	std::string tar = PathAppend(mCurrPath.string(), target.string());
	std::string LastDir = PathAppend(tar, "..\\");
	// 先判断是不是路径，这里顺序不能变更，因为LastDir必须要检测
	if (exists(LastDir) && exists(src))
	{
		// 不允许对上层或本层路径进行重命名
		if (src.length() <= mCurrPath.string().length())
		{
			mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
			return false;
		}
		rename(src, tar);
		mClientEvent.statusCode = FTPStat_FileOperationComfirm;
		return true;
	}

	// 再判断是不是文件
	src.pop_back();
	tar.pop_back();
	std::ifstream fin(src);
	if (!fin.is_open())
	{
		fin.close();
		mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
		return false;
	}
	fin.close();
	rename(src, tar);
	mClientEvent.statusCode = FTPStat_FileOperationComfirm;
	return true;
}



bool FTPServer::MakeDir(const FilePath & path)
{
	std::string Dir = PathAppend(mCurrPath.string(), path.string());
	std::string LastDir = PathAppend(Dir, "..\\");
	if (exists(LastDir))
	{
		create_directory(Dir);
		mClientEvent.statusCode = FTPStat_FileOperationComfirm;
		return true;
	}
	else
	{
		mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
		return false;
	}
		
}

bool FTPServer::GetFile(const FilePath & fromFile)
{
	// 这里只检测语法，如果检验通过，交给数据传输套接字进行。
	std::string cwp = mCurrPath.string();
	std::string File = PathAppend(cwp, fromFile.string());
	std::string Dir = PathAppend(File, "..\\");
	File.pop_back();


	if (Dir.length() < cwp.length() || !exists(File))
	{
		mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
		return false;
	}
	
	mClientEvent.statusCode = FTPStat_FileOperationComfirm;
	return true;
}



bool FTPServer::Delete(const FilePath & p)
{
	std::string str = PathAppend(mCurrPath.string(), p.string());
	// 先检查是不是路径
	if (exists(str))
	{
		// 检查是不是同级路径
		if (str == mCurrPath)
		{
			// 检查是不是根路径，不允许被删除
			if (str == mRootPath)
			{
				mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
				return false;
			}
			remove(str);
			// 删除后返回上层
			return SetUpDir();
		}
		// 检查是不是上级路径，是的话禁止操作
		if (str.length() < mCurrPath.string().length())
		{
			mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
			return false;
		}
		remove(str);
		mClientEvent.statusCode = FTPStat_FileOperationComfirm;
		return true;
	}
	// 然后检查是不是文件
	str.pop_back();
	std::ifstream fin(str);
	if (!fin.is_open())
	{
		fin.close();
		mClientEvent.statusCode = FTPStat_NoSuchDirectoryOrFile;
		return false;
	}
	fin.close();
	remove(str);
	mClientEvent.statusCode = FTPStat_FileOperationComfirm;
	return true;
}




FTPServer::~FTPServer()
{
	mLog.close();
}

void FTPServer::Parse(char * buffer, uint32_t bufferCount)
{
	buffer[bufferCount - 1] = '\0';
	std::istringstream iss(buffer);
	std::string str;
	iss >> str;
	// 仅登陆和离开操作不需要检测是否登陆
	if (str == "user")
	{
		strcpy_s(mClientEvent.userEvent, "login");
		if (!(iss >> mClientEvent.str1 >> mClientEvent.str2))
		{
			mClientEvent.statusCode = FTPStat_ArgumentSyntaxError;

		}
		else if (LogIn(mClientEvent.str1, mClientEvent.str2))
		{
			mClientEvent.statusCode = FTPStat_UserLoggedIn;
			mClientCtrlInfos[mClientEvent.clientIdx].loggined = true;

		}
		else
			mClientEvent.statusCode = FTPStat_InvalidUserNameOrPassWord;
	}
	// 下面这些操作都要先检查是否登陆
	else if (mClientCtrlInfos[mClientEvent.clientIdx].loggined)
	{
		if (str == "cdup")
		{
			strcpy_s(mClientEvent.userEvent, "cdup");
			if (!ParseRest(iss))
				return;
			SetUpDir();
		}
		else if (str == "cd")
		{
			strcpy_s(mClientEvent.userEvent, "cd");
			if (!ParsePath(iss, 1))
				strcpy_s(mClientEvent.str1, ".");
			if (!ParseRest(iss))
				return;
			SetCurrDir(mClientEvent.str1);
		}
		else if (str == "delete")
		{
			strcpy_s(mClientEvent.userEvent, "delete");
			ParsePath(iss, 1);
			if (!ParseRest(iss))
				return;
			Delete(mClientEvent.str1);
			
		}
		else if (str == "get")
		{
			strcpy_s(mClientEvent.userEvent, "get");
			if (!ParsePath(iss, 1))
				return;
			if (!ParseRest(iss))
				return;
			GetFile(mClientEvent.str1);
		}
		else if (str == "mkdir")
		{
			strcpy_s(mClientEvent.userEvent, "mkdir");
			ParsePath(iss, 1);
			if (!ParseRest(iss))
				return;
			MakeDir(mClientEvent.str1);
		}
		else if (str == "rename")
		{
			strcpy_s(mClientEvent.userEvent, "rename");
			ParsePath(iss, 1);
			ParsePath(iss, 2);
			Rename(mClientEvent.str1, mClientEvent.str2);

		}
		else if (str == "put")
		{
			strcpy_s(mClientEvent.userEvent, "put");
			if (!ParsePath(iss, 1))
				return;
			if (!ParseRest(iss))
				return;
			mClientEvent.statusCode = FTPStat_FileOperationComfirm;
		}
	}
	else
	{
		mClientEvent.statusCode = FTPStat_UserNotLogIn;
	}
	
	
}

bool FTPServer::ParsePath(std::istringstream & iss, int EventStr)
{
	if (EventStr < 1 || EventStr > 2)
		return false;
	while (isblank(iss.peek()))
		iss.get();
	if (iss.eof())
	{
		mClientEvent.statusCode = FTPStat_ArgumentSyntaxError;
		return false;
	}
	// 区分是否有冒号
	if (iss.peek() == '\"')
	{
		iss.get();
		if (EventStr == 1)
			iss.getline(mClientEvent.str1, MAX_PATH, '\"');
		else if (EventStr == 2)
			iss.getline(mClientEvent.str2, MAX_PATH, '\"');
	}
	else
	{
		std::string str;
		iss >> str;
		if (EventStr == 1)
			strcpy_s(mClientEvent.str1, str.c_str());
		else if (EventStr == 2)
			strcpy_s(mClientEvent.str2, str.c_str());
	}
	return true;
}

bool FTPServer::ParseRest(std::istringstream & iss)
{
	while (isblank(iss.peek()))
		iss.get();
	if (!iss.eof())
	{
		mClientEvent.statusCode = FTPStat_ArgumentSyntaxError;
		return false;
	}
	return true;
}

std::string FTPServer::PathAppend(std::string curr, std::string extend)
{
	while (extend.length() > 0)
	{
		// 先检查上一级目录
		size_t partion = min(extend.find_first_of("/"), extend.find_first_of("\\"));
		if (extend[0] == '.' && extend[1] == '.')
		{
			if (partion == 2)
				extend.erase(0, 3);
			else if (partion == std::string::npos)
				extend.clear();
			// 删除反斜杠
			curr.pop_back();
			// 查找最近的反斜杠，若没有，则已经退过头了，返回空的字符串
			size_t pos;
			if ((pos = curr.find_last_of('\\')) == std::string::npos)
				return std::string();
			// 删除反斜杠后面的内容
			curr.erase(pos + 1);
		}
		// 然后检查同级目录
		else if (extend[0] == '.')
		{
			if (partion == 1)
				extend.erase(0, 2);
			else if (partion == std::string::npos)
				extend.clear();
		}
		// 最后检查下一层
		else if (partion != std::string::npos)
		{
			curr += extend.substr(0, partion) + '\\';
			extend.erase(0, partion + 1);
		}
		else
		{
			curr += extend + '\\';
			extend.clear();
			break;
		}
	}
	return curr;
}



void FTPServer::ShowHostIP()
{
	// 服务器需要先获取本地IP地址
	char buffer[256]{};
	char ipAddr[16]{};
	// 取得本地主机名称
	::gethostname(buffer, 256);
	// 通过主机名得到IP地址信息
	ADDRINFO* info;
	ADDRINFO hints{};
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	::getaddrinfo(buffer, nullptr, &hints, &info);
	// 打印出所有IP地址
	while (info)
	{
		sockaddr_in* pSin = (sockaddr_in*)info->ai_addr;
		sprintf_s(buffer, "主机IP地址: %s", inet_ntop(AF_INET, &pSin->sin_addr, ipAddr, sizeof ipAddr));
		Console::WriteLine(buffer);
		info = info->ai_next;
	}
	freeaddrinfo(info);
}

char* FTPServer::ShowServerTime(char * outBuffer, uint32_t bufferCount)
{
	time_t tTime;
	tm systemTime;		//系统时间
	time(&tTime);
	localtime_s(&systemTime, &tTime);
	sprintf_s(outBuffer, bufferCount, "%d/%d/%d %d:%02d:%02d", systemTime.tm_year + 1900, systemTime.tm_mon + 1,
		systemTime.tm_mday, systemTime.tm_hour, systemTime.tm_min, systemTime.tm_sec);
	return outBuffer;
}

void FTPServer::WriteLog()
{
	std::string str, event;
	char logStr[100]{};
	char time[20]{};
	char ipAddr[24]{};
	auto& info = mClientCtrlInfos[mClientEvent.clientIdx];
	// 时间|IP地址|(事件)[路径]
	sprintf_s(logStr, "%-19s|%-13s|(%s)", ShowServerTime(time, sizeof time),
		inet_ntop(AF_INET, &info.ipAddr.sin_addr, ipAddr, sizeof ipAddr),
		mClientEvent.userEvent);
	str = logStr;
	event = mClientEvent.userEvent;
	// 具体事件
	if (event == "get" || event == "cd" || event == "mkdir" ||
		event == "delete" || event == "put" || event == "get")
		str = str + "[" + mClientEvent.str1 + "]";
	else if (event == "rename")
		str = str + "[" + mClientEvent.str1 + "][" + mClientEvent.str2 + "]";

	Console::WriteLine(str.c_str());
	mLog << str << std::endl;
}

void FTPServer::SendCurrDirectories()
{
	// 遍历所有路径
	char buffer[1024]{};
	std::string cwp = mCurrPath.string(), rp = mRootPath.string();
	directory_iterator iter(mCurrPath);
	std::vector<std::string> directories;
	for (auto& p : iter)
	{
		directories.push_back(p.path().string().substr(cwp.length()));
		// 是路径的话用\\标注
		if (is_directory(p))
			directories.back() += '\\';
	}


	// 格式: [目录数][当前路径]
	int numDirectories = directories.size();
	*reinterpret_cast<int*>(buffer) = numDirectories;
	strcpy_s(buffer + 4, 1020, cwp.c_str() + rp.length());
	if (!buffer[4] || !strcmp(buffer + 4, "\\"))
		strcpy_s(buffer + 4, 3, ".\\");

	send(mClientCtrlInfos[mClientEvent.clientIdx].socket, buffer, 1024, 0);


	// 逐个路径发送
	// 格式: [路径/文件]
	memset(buffer, 0, 1024);
	for (int i = 0; i < numDirectories; ++i)
	{
		strcpy_s(buffer, 1024, directories[i].c_str());
		send(mClientCtrlInfos[mClientEvent.clientIdx].socket, buffer, 1024, 0);
	}

}

void FTPServer::SendFile(SOCKET s, std::string file)
{
	char buffer[1024]{};
	std::string str = PathAppend(mCurrPath.string(), file);
	str.pop_back();
	// 打开文件
	std::ifstream fin(str, std::ios::binary | std::ios::in);
	fin.seekg(0, std::ios::end);
	int32_t fileSize = (int32_t)fin.tellg();
	fin.seekg(0, std::ios::beg);
	*reinterpret_cast<int *>(buffer) = fileSize;
	strcpy_s(buffer + 4, 1020, str.c_str() + str.find_last_of('\\') + 1);

	send(s, buffer, 1024, 0);
	
	if (fileSize == 0)
	{
		fin.close();
		return;
	}
		

	int32_t totalPacks = (fileSize + 1019) / 1020;
	for (int i = 0; i < totalPacks - 1; ++i)
	{
		fin.read(buffer, 1024);
		while (send(s, buffer, 1024, 0) <= 0)
			continue;
	}
	int32_t rest = fileSize % 1024 ? fileSize % 1024 : 1024;
	fin.read(buffer, rest);
	while (send(s, buffer, rest, 0) <= 0)
		continue;
	fin.close();
}

void FTPServer::RecvFile(SOCKET s, std::string file)
{
	

	char buffer[1024]{};
	file = PathAppend(mCurrPath.string(), file);
	file.pop_back();

	recv(s, buffer, 1024, 0);
	int32_t fileSize = *reinterpret_cast<int32_t*>(buffer);

	std::ofstream fout(file, std::ios::out | std::ios::binary);
	if (fileSize == 0)
	{
		fout.close();
		return;
	}
	int32_t currSize = 0, packSize = 0;
	// 先接下所有分段
	while (currSize < fileSize)
	{
		while ((packSize = recv(s, buffer, 1024, 0)) <= 0)
			continue;
		// 减速保安全
		
		fout.write(buffer, packSize);
		currSize += packSize;
	}
	fout.close();
}





void FTPServer::UserCtrlConnect()
{
	if (::WSAWaitForMultipleEvents(1, &mhCtrl, FALSE, 100UL, FALSE) - WSA_WAIT_EVENT_0 == 0)
	{
		// 获取到来的通知消息
		WSANETWORKEVENTS netWorkEvent;
		::WSAEnumNetworkEvents(msCtrl, mhCtrl, &netWorkEvent);
		uint32_t numUsers = mClientCtrlInfos.size();
		if ((netWorkEvent.lNetworkEvents & FD_ACCEPT) && netWorkEvent.iErrorCode[FD_ACCEPT_BIT] == 0)
		{
			if (numUsers > WSA_MAXIMUM_WAIT_EVENTS)
			{
				Console::WriteLine("Too Many Connections!");
			}
			else
			{
				int addrlen = sizeof(sockaddr_in);

				mhClientCtrls.push_back(WSACreateEvent());
				mClientCtrlInfos.push_back({});
				mClientCtrlInfos[numUsers].socket = ::accept(msCtrl, (sockaddr*)&mClientCtrlInfos[numUsers].ipAddr, &addrlen);
				mClientCtrlInfos[numUsers].loggined = false;
				::WSAEventSelect(mClientCtrlInfos[numUsers].socket, mhClientCtrls[numUsers], FD_READ | FD_CLOSE | FD_WRITE);

				// 发送状态码
				strcpy_s(mClientEvent.userEvent, "open");
				mClientEvent.statusCode = FTPStat_UserConnected;
				WriteLog();
				::send(mClientCtrlInfos[numUsers].socket, reinterpret_cast<char*>(&mClientEvent.statusCode), 1024, 0);
			}
		}
	}
}

void FTPServer::UserDataConnect()
{
	if (::WSAWaitForMultipleEvents(1, &mhData, FALSE, 100UL, FALSE) - WSA_WAIT_EVENT_0 == 0)
	{
		// 获取到来的通知消息
		WSANETWORKEVENTS netWorkEvent;
		::WSAEnumNetworkEvents(msData, mhData, &netWorkEvent);
		uint32_t numUsers = mhClientDatas.size();
		if ((netWorkEvent.lNetworkEvents & FD_ACCEPT) && netWorkEvent.iErrorCode[FD_ACCEPT_BIT] == 0)
		{
			if (numUsers > WSA_MAXIMUM_WAIT_EVENTS)
			{
				Console::WriteLine("Too Many Connections!");
			}
			else
			{
				int addrlen = sizeof(sockaddr_in);

				mhClientDatas.push_back(WSACreateEvent());
				mClientDataInfos.push_back({});
				mClientDataInfos[numUsers].socket = ::accept(msData, (sockaddr*)&mClientDataInfos[numUsers].ipAddr, &addrlen);
				mClientDataInfos[numUsers].loggined = true;
				mDataThreads.push_back({});
				::WSAEventSelect(mClientDataInfos[numUsers].socket, mhClientDatas[numUsers], FD_READ | FD_CLOSE | FD_WRITE);
			}
		}
	}
}

void FTPServer::UserSendCtrlMessage()
{
	uint32_t numUsers = mClientCtrlInfos.size();
	// 对所有事件只等待500ms（只要有一个触发）
	int nIndex = ::WSAWaitForMultipleEvents(numUsers, mhClientCtrls.data(), FALSE, 500UL, FALSE);
	// 对每个事件调用WSAWaitForMultipleEvents函数，以便确定它的状态
	nIndex = nIndex - WSA_WAIT_EVENT_0;
	for (size_t i = nIndex; i< numUsers; i++)
	{
		// 检验该事件对象是否触发
		nIndex = ::WSAWaitForMultipleEvents(1, &mhClientCtrls[i], TRUE, 500UL, FALSE);
		if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)
		{
			continue;
		}
		else
		{
			// 获取到来的通知消息
			WSANETWORKEVENTS netWorkEvent;
			::WSAEnumNetworkEvents(mClientCtrlInfos[i].socket, mhClientCtrls[i], &netWorkEvent);
			if ((netWorkEvent.lNetworkEvents & FD_READ) && netWorkEvent.iErrorCode[FD_READ_BIT] == 0)			// 处理FD_READ通知消息
			{
				char buffer[1024]{};
				int nRecv = ::recv(mClientCtrlInfos[i].socket, buffer, sizeof buffer, 0);
				if (nRecv > 0)
				{
					// 标记客户端索引，然后处理消息
					mClientEvent.clientIdx = i;
					Parse(buffer, sizeof buffer);

					// 将状态码塞入发送队列
					memset(buffer, 0, sizeof buffer);
					*reinterpret_cast<int*>(buffer) = mClientEvent.statusCode;
					send(mClientCtrlInfos[i].socket, buffer, sizeof buffer, 0);


					switch (mClientEvent.statusCode)
					{
						// 对用户登陆、文件操作还需要发送当前目录
					case FTPStat_UserLoggedIn: 
					case FTPStat_FileOperationComfirm:
						SendCurrDirectories();
					break;
					}

					// 对已经确认的事件打日志
					if (mClientEvent.statusCode / 100 == 2)
						WriteLog();

				}
			}
			else if (netWorkEvent.lNetworkEvents & FD_CLOSE)		// 处理FD_CLOSE通知消息
			{
				::closesocket(mClientCtrlInfos[i].socket);


				strcpy_s(mClientEvent.userEvent, "close");
				mClientEvent.statusCode = FTPStat_UserClosed;
				WriteLog();

				// 清除该用户
				mClientCtrlInfos.erase(mClientCtrlInfos.begin() + i, mClientCtrlInfos.begin() + i + 1);
				mhClientCtrls.erase(mhClientCtrls.begin() + i, mhClientCtrls.begin() + i + 1);
				numUsers--;
			}
			else if (netWorkEvent.lNetworkEvents & FD_WRITE)		// 处理FD_WRITE通知消息
			{
				
			}

		}
	}
}

void FTPServer::UserSendDataMessage()
{
	uint32_t numUsers = mClientDataInfos.size();
	// 对所有事件只等待500ms（只要有一个触发）
	int nIndex = ::WSAWaitForMultipleEvents(numUsers, mhClientDatas.data(), FALSE, 500UL, FALSE);
	// 对每个事件调用WSAWaitForMultipleEvents函数，以便确定它的状态
	nIndex = nIndex - WSA_WAIT_EVENT_0;
	for (size_t i = nIndex; i< numUsers; i++)
	{
		// 检验该事件对象是否触发
		nIndex = ::WSAWaitForMultipleEvents(1, &mhClientDatas[i], TRUE, 500UL, FALSE);
		if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)
		{
			continue;
		}
		else
		{
			// 获取到来的通知消息
			WSANETWORKEVENTS netWorkEvent;
			::WSAEnumNetworkEvents(mClientDataInfos[i].socket, mhClientDatas[i], &netWorkEvent);
			if ((netWorkEvent.lNetworkEvents & FD_READ) && netWorkEvent.iErrorCode[FD_READ_BIT] == 0)			// 处理FD_READ通知消息
			{
				char buffer[1024]{};
				int nRecv = ::recv(mClientDataInfos[i].socket, buffer, sizeof buffer, 0);
				if (nRecv > 0)
				{
					std::istringstream iss(buffer);
					std::string str, file;
					iss >> str;
					
					while (isblank(iss.peek()))
						iss.get();
					// 区分是否有冒号
					if (iss.peek() == '\"')
					{
						iss.get();
						getline(iss, file, '\"');
					}
					else
						iss >> file;
					if (str == "get")
					{
						mDataThreads[i] = std::thread(&FTPServer::SendFile, this, mClientDataInfos[i].socket, file);
						mDataThreads[i].detach();
					}
					else if (str == "put")
					{
						// 屏蔽FD_READ信息
						::WSAEventSelect(mClientDataInfos[i].socket, mhClientDatas[i], FD_CLOSE | FD_WRITE);
						mDataThreads[i] = std::thread(&FTPServer::RecvFile, this, mClientDataInfos[i].socket,
							file.substr(max(file.find_last_of('\\') + 1, file.find_last_of('/') + 1)));
						mDataThreads[i].detach();
					}

				}
			}
			else if (netWorkEvent.lNetworkEvents & FD_CLOSE)		// 处理FD_CLOSE通知消息
			{
				::closesocket(mClientDataInfos[i].socket);

				// 清除该用户
				mClientDataInfos.erase(mClientDataInfos.begin() + i, mClientDataInfos.begin() + i + 1);
				mhClientDatas.erase(mhClientDatas.begin() + i, mhClientDatas.begin() + i + 1);
				mDataThreads.erase(mDataThreads.begin() + i, mDataThreads.begin() + i + 1);
				
				numUsers--;
			}
			else if (netWorkEvent.lNetworkEvents & FD_WRITE)		// 处理FD_WRITE通知消息
			{

			}

		}
	}
}



bool FTPCreateServer(IServer ** pOut)
{
	if (gServer || !pOut)
		return false;
	*pOut = gServer = new FTPServer;
	return true;
}

bool FTPCloseServer()
{
	if (gServer)
	{
		delete gServer;
		gServer = nullptr;
		return true;
	}
	else
		return false;
}
