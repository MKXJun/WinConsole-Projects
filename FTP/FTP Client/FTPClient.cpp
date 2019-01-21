#include "FTPClient.h"
#include <sstream>
#include <fstream>
#include <ctime>
#include <filesystem>

using namespace std::experimental::filesystem;


FTPClient::FTPClient()
	: mConsole(Console::Get()),
	msCtrl{},
	mServerAddr{ AF_INET },
	mFTPCtrlStatusCode{},
	mFTPDataStatusCode{},
	mRecvBuffer(1024),
	mCtrlBuffer(1024),
	mtData()
{
}

void FTPClient::Run()
{
	InitWindow();
	
	ConsoleInput();

}

void FTPClient::OpenCtrl(std::istringstream & iss)
{
	std::string str;
	char ipStr[17];
	iss >> str;
	// 检验ip地址的合法性
	int a, b, c, d, port = 0;
	if (sscanf_s(str.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4 ||
		a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255 ||
		sprintf_s(ipStr, "%d.%d.%d.%d", a, b, c, d) != str.length())
	{
		Console::WriteLine("[501]Error:IP地址不合法!");
		RollLine();
		
		return;
	}
	inet_pton(AF_INET, str.c_str(), &mServerAddr.sin_addr);

	if (msCtrl == 0)
	{
		// 检验端口号
		if (iss >> port && port >= 0 && port <= 65535)
			mServerAddr.sin_port = htons(port);
		else if (iss.fail() && !iss.eof() || port < 0 || port > 65535)
		{
			Console::WriteLine("[501]Error:端口号不合法!");
			RollLine();
			return;
		}
		else
			mServerAddr.sin_port = htons(21);

		// 检验冗余项是否存在
		if (!ParseRest(iss))
		{
			Console::WriteLine("[501]Error:语法错误!");
			RollLine();
			return;
		}
		// 创建SOCKET
		msCtrl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (msCtrl == INVALID_SOCKET)
		{
			MessageBoxA(NULL, "SOCKET创建失败", "错误", MB_OK);
			system("PAUSE");
			exit(EXIT_FAILURE);
		}

		// 连接不上的话直接本地描述状态码
		if (connect(msCtrl, (sockaddr*)&mServerAddr, sizeof(mServerAddr)) == -1)
		{
			// 需要关闭套接字
			closesocket(msCtrl);
			msCtrl = 0;
			Console::WriteLine("[421]Error:服务器不可用!");
			RollLine();
			return;
		}

		
		Console::WriteBuffer(2, 1, (std::string("服务器地址: ") + ipStr).c_str());
		
		GetAndWriteFTPStatusCode();
		
	}
	else
	{
		Console::WriteLine("[502]Error:需要关闭(close)当前服务器连接才能使用!");
		RollLine();
	}
}

void FTPClient::CloseCtrl(std::istringstream & iss)
{
	// 检验冗余项是否存在
	if (!ParseRest(iss))
	{
		Console::WriteLine("[501]Error:语法错误!");
		RollLine();
		return;
	}
	if (msCtrl == 0)
	{
		Console::WriteLine("[502]Error:当前未连接到服务器!");
		RollLine();
		return;
	}
	closesocket(msCtrl);
	// 擦掉上层界面
	for (int i = 1; i < 29; ++i)
		Console::WriteBuffer(2, i, "                                                                            ");
	msCtrl = 0;
	
	Console::WriteLine("[221]Confirm:已关闭与服务器的连接!");
	RollLine();
}

void FTPClient::BeginTransfer(const std::string& cmdOrFile)
{
	msData = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (msData == INVALID_SOCKET)
	{
		MessageBoxA(NULL, "SOCKET创建失败", "错误", MB_OK);
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	// 连接不上的话直接本地描述状态码
	mServerAddr.sin_port = htons(20);
	connect(msData, (sockaddr*)&mServerAddr, sizeof(mServerAddr));
	// 再发一遍给数据
	send(msData, mCtrlBuffer.data(), mCtrlBuffer.size(), 0);

	char buffer[1024]{};
	std::ofstream fout;
	clock_t curr = clock();
	int32_t lastSecSize = 0, fileSize = 0, lastSecPack = 0;
	char outBuffer1[77]{}, outBuffer2[77]{};
	if (cmdOrFile == "get")
	{
		// 格式: [文件大小][文件名]
		recv(msData, buffer, 1024, 0);
		fileSize = *reinterpret_cast<int32_t*>(buffer);
		std::string fileName = "SaveData\\";
		fileName += &buffer[4];
		fout.open(fileName, std::ios::out | std::ios::binary);
		
		Console::WriteBuffer(2, 26, "****************************************************************************");
		Console::WriteBuffer(2, 27, (std::string("当前下载文件: ") + (&buffer[4]) + 
			std::string(62 -  strlen(&buffer[4]), ' ')).c_str());

		if (fileSize == 0)
		{
			Console::WriteBuffer(2, 28, "下载已完成!                                                                 ");
			closesocket(msData);
			fout.close();
			return;
		}

		int currSize = 0, packSize = 0;
		// 先接下所有包
		while (currSize < fileSize)
		{
			while ((packSize = recv(msData, buffer, 1024, 0)) <= 0)
				continue;
			fout.write(buffer, packSize);
			currSize += packSize;
			// 减速保安全
			
			sprintf_s(outBuffer1, "进度: %dkb/%dkb ", currSize / 1024, fileSize / 1024);
			Console::WriteBuffer(2, 28, outBuffer1);
			if (clock() - curr > 1000)
			{
				sprintf_s(outBuffer2, "网速: %dkb/s      ", (currSize - lastSecSize) / 1024);
				lastSecSize = currSize;
				curr = clock();
			}
			Console::WriteBuffer(36, 28, outBuffer2);
		}

		fout.close();
		Console::WriteBuffer(2, 28, "下载已完成!                                                                 ");
		
		// 关闭套接字
		closesocket(msData);
	}
	else
	{
		// 格式: [文件大小]
		std::ifstream fin(cmdOrFile, std::ios::in | std::ios::binary);
		fin.seekg(0, std::ios::end);
		int32_t fileSize = (int32_t)fin.tellg();
		fin.seekg(0, std::ios::beg);
		*reinterpret_cast<int32_t*>(buffer) = fileSize;

		send(msData, buffer, 1024, 0);

		Console::WriteBuffer(2, 26, "****************************************************************************");
		Console::WriteBuffer(2, 27, (std::string("当前上传文件: ") + (&buffer[4]) +
			std::string(62 - strlen(&buffer[4]), ' ')).c_str());

		if (fileSize == 0)
		{
			Console::WriteBuffer(2, 28, "上传已完成!");
			fin.close();
			closesocket(msData);
			return;
		}

		int32_t totalPacks = (fileSize + 1023) / 1024;
		for (int i = 0; i < totalPacks - 1; ++i)
		{
			fin.read(buffer, 1024);

			while (send(msData, buffer, 1024, 0) <= 0)
				continue;
			
			sprintf_s(outBuffer1, "进度: %dkb/%dkb ", i, fileSize / 1024);
			Console::WriteBuffer(2, 28, outBuffer1);

			if (clock() - curr > 1000)
			{
				sprintf_s(outBuffer2, "网速: %dkb/s      ", i - lastSecPack);
				lastSecPack = i;
				curr = clock();
			}
			Console::WriteBuffer(36, 28, outBuffer2);
		}

		int32_t rest = fileSize % 1024 ? fileSize % 1024 : 1024;
		fin.read(buffer, rest);
		while (send(msData, buffer, rest, 0) <= 0)
			continue;
		fin.close();
		
		Console::WriteBuffer(2, 28, "上传已完成!                                                                 ");

		closesocket(msData);
	}

}





void FTPClient::ShowCurrPath()
{
	recv(msCtrl, mRecvBuffer.data(), mRecvBuffer.size(), 0);
	int32_t numDir = *reinterpret_cast<int32_t*>(mRecvBuffer.data());
	mDirectories.resize(numDir);
	mCurrPath = mRecvBuffer.data() + 4;

	for (int i = 0; i < numDir; ++i)
	{
		recv(msCtrl, mRecvBuffer.data(), mRecvBuffer.size(), 0);
		mDirectories[i] = mRecvBuffer.data();
	}

	// 先清空界面
	for (int i = 2; i <= 25; ++i)
		Console::WriteBuffer(2, i, "                                                                           ");
	// 输出当前路径
	Console::WriteBuffer(2, 2, (std::string("当前路径: ") + mCurrPath).c_str());
	Console::WriteBuffer(2, 3, "****************************************************************************");
	// 输出前22个目录
	for (int i = 0; i < min(22, (int)mDirectories.size()); ++i)
		Console::WriteBuffer(2, i + 4, mDirectories[i].c_str());
}

void FTPClient::ShowHelp(std::istringstream& iss)
{
	static const char * helps[] = {
		// 0
		"有关某个命令的详细信息，请键入 help 命令名; 要查看剩余的命令，请键入 help 2 ",
		"注意: 若路径存在空格，可以这样: \"Hello World\\\"                           ",
		"bye                     关闭与服务器的连接                                  ",
		"cd                      跳转并显示当前目录                                  ",
		"cdup                    跳转到上一级目录                                    ",
		"delete                  删除服务器端指定文件                                ",
		"get                     从服务器端获取指定文件                              ",
		"help                    获取所有命令或指定命令的解释                        ",
		"mkdir                   在服务器端新建目录                                  ",
		"open                    建立与服务端的连接                                  ",
		"put                     将本地一个文件传输至服务器端                        ",
		"quit                    关闭与服务器的连接                                  ",
		"rename                  重命名/移动服务器端的文件夹/文件                    ",
		"user                    使用账户登录服务器以获取操作权限                    ",
		// 14
		"跳转并显示当前目录，例如:                                                   ",
		"cd                      显示当前路径下的文件/文件夹                         ",
		"cd ./                   同上                                                ",
		"cd .                    同上                                                ",
		"cd dir/                 跳转至文件夹dir内并显示                             ",
		"cd dir                  同上                                                ",
		"cd ../                  跳转至上一层文件夹并显示                            ",
		"cd ..                   同上                                                ",
		// 22
		"删除服务器端指定文件，例如:                                                 ",
		"delete ./               删除该层所有文件，返回上一层                        ",
		"delete .                同上                                                ",
		"delete dir/             删除dir文件夹及里面的所有文件                       ",
		"delete dir              同上                                                ",
		"delete file.txt         删除file.txt                                        ",
		// 28
		"从服务器端获取指定文件，例如:                                               ",
		"get file.txt            获取file.txt到本地                                  ",
		"get dir/a.txt           获取dir路径下的a.txt到本地                          ",
		// 31
		"在服务器端新建目录，例如:                                                   ",
		"mkdir dir/              在当前路径下新建dir文件夹                           ",
		"mkdir dir               同上                                                ",
		"mkdir dir1/dir2         在dir1文件夹内新建dir2文件夹                        ",
		// 35
		"建立与服务端的连接, 例如:                                                   ",
		"open 127.0.0.1          需要本地开启服务器才能连接，默认连接端口21          ",
		"open 192.168.1.2        连接指定服务器，默认连接端口为21                    ",
		"open 192.168.1.2 99     连接指定服务器，使用端口号99                        ",
		// 39
		"将本地一个文件传输至服务器端当前位置, 例如:                                 ",
		"put file.txt            将位于SaveData文件夹的file.txt传输到服务端当前位置  ",
		"put E:/file.txt         将位于E盘的file.txt传输到服务端当前位置             ",
		// 42
		"重命名/移动服务器端的文件夹/文件，例如:                                     ",
		"rename dir1 dir2        将文件夹dir1改名为dir2                              ",
		"rename a.txt b.txt      将文件a.txt改名为b.txt                              ",
		"rename a.txt dir/a.txt  将文件a.txt移动到dir内                              ",
		// 46
		"使用账户登录服务器以获取操作权限，例如:                                     ",
		"user account password   使用账户account和密码password登陆当前连接的服务器   ",
		// 48
		"                                                                            "
	};

	std::string str;
	// 先清掉空白来判断
	if (ParseRest(iss) || (iss >> str) && str == "1")
	{
		for (int i = 0; i < 8; ++i)
		{
			Console::WriteLine(helps[i]);
			RollLine();
		}	
		return;
	}
	else if (str == "2")
	{
		Console::WriteLine(helps[0]);
		RollLine();
		Console::WriteLine(helps[1]);
		RollLine();
		for (int i = 8; i < 14; ++i)
		{
			Console::WriteLine(helps[i]);
			RollLine();
		}
		return;
	}

	if (!ParseRest(iss))
	{
		Console::WriteLine("[501]Error:语法错误!");
		RollLine();
		return;
	}
		
	int beg = 0, ed = 0;
	if (str == "bye" || str == "quit")
		beg = 2, ed = 3;
	else if (str == "cdup")
		beg = 4, ed = 5;
	else if (str == "cd")
		beg = 14, ed = 22;
	else if (str == "delete")
		beg = 22, ed = 28;
	else if (str == "get")
		beg = 28, ed = 31;
	else if (str == "mkdir")
		beg = 31, ed = 35;
	else if (str == "open")
		beg = 35, ed = 39;
	else if (str == "put")
		beg = 39, ed = 42;
	else if (str == "rename")
		beg = 42, ed = 46;
	else if (str == "user")
		beg = 46, ed = 48;
	
	while (beg < ed)
	{
		Console::WriteLine(helps[beg++]);
		RollLine();
	}
}



void FTPClient::Parse()
{
	mCtrlBuffer[1023] = '\0';
	std::istringstream iss(mCtrlBuffer.data());
	std::string str;
	iss >> str;

	if (str == "open")
	{
		OpenCtrl(iss);
	}
	else if (str == "close" || str == "bye")
	{
		CloseCtrl(iss);
	}
	else if (str == "help" || str == "?")
	{
		ShowHelp(iss);
	}
	else if (str == "user")
	{
		send(msCtrl, mCtrlBuffer.data(), mCtrlBuffer.size(), 0);
		if (GetAndWriteFTPStatusCode() == FTPStat_UserLoggedIn)
			ShowCurrPath();
	}
	else if (str == "cd" || str == "cdup" || str == "delete" || str == "mkdir" 
		|| str == "rename")
	{
		send(msCtrl, mCtrlBuffer.data(), mCtrlBuffer.size(), 0);
		if (GetAndWriteFTPStatusCode() == FTPStat_FileOperationComfirm)
			ShowCurrPath();
	}
	else if (str == "get")
	{
		send(msCtrl, mCtrlBuffer.data(), mCtrlBuffer.size(), 0);
		if (GetAndWriteFTPStatusCode() == FTPStat_FileOperationComfirm)
		{
			ShowCurrPath();
			mtData = std::thread(&FTPClient::BeginTransfer, this, str);
			mtData.detach();
		}
	}
	else if (str == "put")
	{
		while (isblank(iss.peek()))
			iss.get();
		// 区分是否有冒号
		if (iss.peek() == '\"')
		{
			iss.get();
			std::getline(iss, str, '\"');
		}
		else
			iss >> str;

		if (exists(str) || exists("SaveData\\" + str))
		{
			send(msCtrl, mCtrlBuffer.data(), mCtrlBuffer.size(), 0);
			if (GetAndWriteFTPStatusCode() == FTPStat_FileOperationComfirm)
			{
				ShowCurrPath();
				mtData = std::thread(&FTPClient::BeginTransfer, this, exists(str) ? str : "SaveData\\" + str);
				mtData.detach();
			}
		}
		else
		{
			Console::WriteFormat(GetFTPStatusString(FTPStat_NoSuchDirectoryOrFile));
			RollLine();
		}
	}
	else
	{
		Console::WriteFormat("%s 不是一个有效的指令!\n", str.c_str());
		RollLine();
	}
	
}

bool FTPClient::ParseRest(std::istringstream & iss)
{
	while (isblank(iss.peek()))
		iss.get();
	return iss.eof();
}

std::string FTPClient::PathAppend(std::string curr, std::string extend)
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


void FTPClient::ConsoleInput()
{
	
	
	while (TRUE)
	{
		mConsole.ReadLine(mCtrlBuffer.data(), mCtrlBuffer.size());
		RollLine();
		
		// 分析语法并处理
		Parse();

		Console::Write("FTP>");
	}
	
}

void FTPClient::RollLine()
{
	static CHAR_INFO outBuffer[3200]{};
	static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (mConsole.GetBuffer(outBuffer) == FALSE)
	{
		MessageBox(nullptr, "请勿擅自修改窗口大小!", "错误", MB_OK);
		exit(1);
	}
	SMALL_RECT rc{ 0, 30, 79, 38 };
	WriteConsoleOutputA(hOut, outBuffer, COORD{ 80, 40 }, COORD{ 0, 31 }, &rc);
	mConsole.SetCursorPos(0, 38);
}

FTPStatusCode FTPClient::GetAndWriteFTPStatusCode()
{
	recv(msCtrl, mRecvBuffer.data(), mRecvBuffer.size(), 0);
	FTPStatusCode code = *reinterpret_cast<FTPStatusCode*>(mRecvBuffer.data());
	Console::Write(GetFTPStatusString(code));
	RollLine();
	return code;
}



void FTPClient::InitWindow()
{
	mConsole.SetWindowSize(80, 40);
	mConsole.SetBufferSize(80, 40);
	// 画主界面外框
	mConsole.WriteBuffer(0, 0, "┌");
	mConsole.WriteBuffer(78, 0, "┐");
	mConsole.WriteBuffer(0, 29, "└");
	mConsole.WriteBuffer(78, 29, "┘");
	for (int i = 2; i <= 76; i += 2)
	{
		mConsole.WriteBuffer(i, 0, "─");
		mConsole.WriteBuffer(i, 29, "─");
	}
	for (int i = 1; i <= 28; ++i)
	{
		mConsole.WriteBuffer(0, i, "│");
		mConsole.WriteBuffer(78, i, "│");
	}


	mConsole.SetCursorPos(0, 38);
	mConsole.Write("FTP>");
}


static IClient* gClient = nullptr;

bool FTPCreateClient(IClient ** pOut)
{
	if (gClient || !pOut)
		return false;
	*pOut = gClient = new FTPClient;
	return true;
}

bool FTPCloseClient()
{
	if (gClient)
	{
		delete gClient;
		gClient = nullptr;
		return true;
	}
	else
		return false;
}

