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
	// ����ip��ַ�ĺϷ���
	int a, b, c, d, port = 0;
	if (sscanf_s(str.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4 ||
		a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255 ||
		sprintf_s(ipStr, "%d.%d.%d.%d", a, b, c, d) != str.length())
	{
		Console::WriteLine("[501]Error:IP��ַ���Ϸ�!");
		RollLine();
		
		return;
	}
	inet_pton(AF_INET, str.c_str(), &mServerAddr.sin_addr);

	if (msCtrl == 0)
	{
		// ����˿ں�
		if (iss >> port && port >= 0 && port <= 65535)
			mServerAddr.sin_port = htons(port);
		else if (iss.fail() && !iss.eof() || port < 0 || port > 65535)
		{
			Console::WriteLine("[501]Error:�˿ںŲ��Ϸ�!");
			RollLine();
			return;
		}
		else
			mServerAddr.sin_port = htons(21);

		// �����������Ƿ����
		if (!ParseRest(iss))
		{
			Console::WriteLine("[501]Error:�﷨����!");
			RollLine();
			return;
		}
		// ����SOCKET
		msCtrl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (msCtrl == INVALID_SOCKET)
		{
			MessageBoxA(NULL, "SOCKET����ʧ��", "����", MB_OK);
			system("PAUSE");
			exit(EXIT_FAILURE);
		}

		// ���Ӳ��ϵĻ�ֱ�ӱ�������״̬��
		if (connect(msCtrl, (sockaddr*)&mServerAddr, sizeof(mServerAddr)) == -1)
		{
			// ��Ҫ�ر��׽���
			closesocket(msCtrl);
			msCtrl = 0;
			Console::WriteLine("[421]Error:������������!");
			RollLine();
			return;
		}

		
		Console::WriteBuffer(2, 1, (std::string("��������ַ: ") + ipStr).c_str());
		
		GetAndWriteFTPStatusCode();
		
	}
	else
	{
		Console::WriteLine("[502]Error:��Ҫ�ر�(close)��ǰ���������Ӳ���ʹ��!");
		RollLine();
	}
}

void FTPClient::CloseCtrl(std::istringstream & iss)
{
	// �����������Ƿ����
	if (!ParseRest(iss))
	{
		Console::WriteLine("[501]Error:�﷨����!");
		RollLine();
		return;
	}
	if (msCtrl == 0)
	{
		Console::WriteLine("[502]Error:��ǰδ���ӵ�������!");
		RollLine();
		return;
	}
	closesocket(msCtrl);
	// �����ϲ����
	for (int i = 1; i < 29; ++i)
		Console::WriteBuffer(2, i, "                                                                            ");
	msCtrl = 0;
	
	Console::WriteLine("[221]Confirm:�ѹر��������������!");
	RollLine();
}

void FTPClient::BeginTransfer(const std::string& cmdOrFile)
{
	msData = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (msData == INVALID_SOCKET)
	{
		MessageBoxA(NULL, "SOCKET����ʧ��", "����", MB_OK);
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	// ���Ӳ��ϵĻ�ֱ�ӱ�������״̬��
	mServerAddr.sin_port = htons(20);
	connect(msData, (sockaddr*)&mServerAddr, sizeof(mServerAddr));
	// �ٷ�һ�������
	send(msData, mCtrlBuffer.data(), mCtrlBuffer.size(), 0);

	char buffer[1024]{};
	std::ofstream fout;
	clock_t curr = clock();
	int32_t lastSecSize = 0, fileSize = 0, lastSecPack = 0;
	char outBuffer1[77]{}, outBuffer2[77]{};
	if (cmdOrFile == "get")
	{
		// ��ʽ: [�ļ���С][�ļ���]
		recv(msData, buffer, 1024, 0);
		fileSize = *reinterpret_cast<int32_t*>(buffer);
		std::string fileName = "SaveData\\";
		fileName += &buffer[4];
		fout.open(fileName, std::ios::out | std::ios::binary);
		
		Console::WriteBuffer(2, 26, "****************************************************************************");
		Console::WriteBuffer(2, 27, (std::string("��ǰ�����ļ�: ") + (&buffer[4]) + 
			std::string(62 -  strlen(&buffer[4]), ' ')).c_str());

		if (fileSize == 0)
		{
			Console::WriteBuffer(2, 28, "���������!                                                                 ");
			closesocket(msData);
			fout.close();
			return;
		}

		int currSize = 0, packSize = 0;
		// �Ƚ������а�
		while (currSize < fileSize)
		{
			while ((packSize = recv(msData, buffer, 1024, 0)) <= 0)
				continue;
			fout.write(buffer, packSize);
			currSize += packSize;
			// ���ٱ���ȫ
			
			sprintf_s(outBuffer1, "����: %dkb/%dkb ", currSize / 1024, fileSize / 1024);
			Console::WriteBuffer(2, 28, outBuffer1);
			if (clock() - curr > 1000)
			{
				sprintf_s(outBuffer2, "����: %dkb/s      ", (currSize - lastSecSize) / 1024);
				lastSecSize = currSize;
				curr = clock();
			}
			Console::WriteBuffer(36, 28, outBuffer2);
		}

		fout.close();
		Console::WriteBuffer(2, 28, "���������!                                                                 ");
		
		// �ر��׽���
		closesocket(msData);
	}
	else
	{
		// ��ʽ: [�ļ���С]
		std::ifstream fin(cmdOrFile, std::ios::in | std::ios::binary);
		fin.seekg(0, std::ios::end);
		int32_t fileSize = (int32_t)fin.tellg();
		fin.seekg(0, std::ios::beg);
		*reinterpret_cast<int32_t*>(buffer) = fileSize;

		send(msData, buffer, 1024, 0);

		Console::WriteBuffer(2, 26, "****************************************************************************");
		Console::WriteBuffer(2, 27, (std::string("��ǰ�ϴ��ļ�: ") + (&buffer[4]) +
			std::string(62 - strlen(&buffer[4]), ' ')).c_str());

		if (fileSize == 0)
		{
			Console::WriteBuffer(2, 28, "�ϴ������!");
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
			
			sprintf_s(outBuffer1, "����: %dkb/%dkb ", i, fileSize / 1024);
			Console::WriteBuffer(2, 28, outBuffer1);

			if (clock() - curr > 1000)
			{
				sprintf_s(outBuffer2, "����: %dkb/s      ", i - lastSecPack);
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
		
		Console::WriteBuffer(2, 28, "�ϴ������!                                                                 ");

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

	// ����ս���
	for (int i = 2; i <= 25; ++i)
		Console::WriteBuffer(2, i, "                                                                           ");
	// �����ǰ·��
	Console::WriteBuffer(2, 2, (std::string("��ǰ·��: ") + mCurrPath).c_str());
	Console::WriteBuffer(2, 3, "****************************************************************************");
	// ���ǰ22��Ŀ¼
	for (int i = 0; i < min(22, (int)mDirectories.size()); ++i)
		Console::WriteBuffer(2, i + 4, mDirectories[i].c_str());
}

void FTPClient::ShowHelp(std::istringstream& iss)
{
	static const char * helps[] = {
		// 0
		"�й�ĳ���������ϸ��Ϣ������� help ������; Ҫ�鿴ʣ����������� help 2 ",
		"ע��: ��·�����ڿո񣬿�������: \"Hello World\\\"                           ",
		"bye                     �ر��������������                                  ",
		"cd                      ��ת����ʾ��ǰĿ¼                                  ",
		"cdup                    ��ת����һ��Ŀ¼                                    ",
		"delete                  ɾ����������ָ���ļ�                                ",
		"get                     �ӷ������˻�ȡָ���ļ�                              ",
		"help                    ��ȡ���������ָ������Ľ���                        ",
		"mkdir                   �ڷ��������½�Ŀ¼                                  ",
		"open                    ���������˵�����                                  ",
		"put                     ������һ���ļ���������������                        ",
		"quit                    �ر��������������                                  ",
		"rename                  ������/�ƶ��������˵��ļ���/�ļ�                    ",
		"user                    ʹ���˻���¼�������Ի�ȡ����Ȩ��                    ",
		// 14
		"��ת����ʾ��ǰĿ¼������:                                                   ",
		"cd                      ��ʾ��ǰ·���µ��ļ�/�ļ���                         ",
		"cd ./                   ͬ��                                                ",
		"cd .                    ͬ��                                                ",
		"cd dir/                 ��ת���ļ���dir�ڲ���ʾ                             ",
		"cd dir                  ͬ��                                                ",
		"cd ../                  ��ת����һ���ļ��в���ʾ                            ",
		"cd ..                   ͬ��                                                ",
		// 22
		"ɾ����������ָ���ļ�������:                                                 ",
		"delete ./               ɾ���ò������ļ���������һ��                        ",
		"delete .                ͬ��                                                ",
		"delete dir/             ɾ��dir�ļ��м�����������ļ�                       ",
		"delete dir              ͬ��                                                ",
		"delete file.txt         ɾ��file.txt                                        ",
		// 28
		"�ӷ������˻�ȡָ���ļ�������:                                               ",
		"get file.txt            ��ȡfile.txt������                                  ",
		"get dir/a.txt           ��ȡdir·���µ�a.txt������                          ",
		// 31
		"�ڷ��������½�Ŀ¼������:                                                   ",
		"mkdir dir/              �ڵ�ǰ·�����½�dir�ļ���                           ",
		"mkdir dir               ͬ��                                                ",
		"mkdir dir1/dir2         ��dir1�ļ������½�dir2�ļ���                        ",
		// 35
		"���������˵�����, ����:                                                   ",
		"open 127.0.0.1          ��Ҫ���ؿ����������������ӣ�Ĭ�����Ӷ˿�21          ",
		"open 192.168.1.2        ����ָ����������Ĭ�����Ӷ˿�Ϊ21                    ",
		"open 192.168.1.2 99     ����ָ����������ʹ�ö˿ں�99                        ",
		// 39
		"������һ���ļ��������������˵�ǰλ��, ����:                                 ",
		"put file.txt            ��λ��SaveData�ļ��е�file.txt���䵽����˵�ǰλ��  ",
		"put E:/file.txt         ��λ��E�̵�file.txt���䵽����˵�ǰλ��             ",
		// 42
		"������/�ƶ��������˵��ļ���/�ļ�������:                                     ",
		"rename dir1 dir2        ���ļ���dir1����Ϊdir2                              ",
		"rename a.txt b.txt      ���ļ�a.txt����Ϊb.txt                              ",
		"rename a.txt dir/a.txt  ���ļ�a.txt�ƶ���dir��                              ",
		// 46
		"ʹ���˻���¼�������Ի�ȡ����Ȩ�ޣ�����:                                     ",
		"user account password   ʹ���˻�account������password��½��ǰ���ӵķ�����   ",
		// 48
		"                                                                            "
	};

	std::string str;
	// ������հ����ж�
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
		Console::WriteLine("[501]Error:�﷨����!");
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
		// �����Ƿ���ð��
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
		Console::WriteFormat("%s ����һ����Ч��ָ��!\n", str.c_str());
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
		// �ȼ����һ��Ŀ¼
		size_t partion = min(extend.find_first_of("/"), extend.find_first_of("\\"));
		if (extend[0] == '.' && extend[1] == '.')
		{
			if (partion == 2)
				extend.erase(0, 3);
			else if (partion == std::string::npos)
				extend.clear();
			// ɾ����б��
			curr.pop_back();
			// ��������ķ�б�ܣ���û�У����Ѿ��˹�ͷ�ˣ����ؿյ��ַ���
			size_t pos;
			if ((pos = curr.find_last_of('\\')) == std::string::npos)
				return std::string();
			// ɾ����б�ܺ��������
			curr.erase(pos + 1);
		}
		// Ȼ����ͬ��Ŀ¼
		else if (extend[0] == '.')
		{
			if (partion == 1)
				extend.erase(0, 2);
			else if (partion == std::string::npos)
				extend.clear();
		}
		// �������һ��
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
		
		// �����﷨������
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
		MessageBox(nullptr, "���������޸Ĵ��ڴ�С!", "����", MB_OK);
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
	// �����������
	mConsole.WriteBuffer(0, 0, "��");
	mConsole.WriteBuffer(78, 0, "��");
	mConsole.WriteBuffer(0, 29, "��");
	mConsole.WriteBuffer(78, 29, "��");
	for (int i = 2; i <= 76; i += 2)
	{
		mConsole.WriteBuffer(i, 0, "��");
		mConsole.WriteBuffer(i, 29, "��");
	}
	for (int i = 1; i <= 28; ++i)
	{
		mConsole.WriteBuffer(0, i, "��");
		mConsole.WriteBuffer(78, i, "��");
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

