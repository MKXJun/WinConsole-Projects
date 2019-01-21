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
		sockaddr_in ipAddr;		// �ͻ���IP��ַ
		SOCKET socket;			// �ͻ����׽���
		bool loggined;			// �ͻ����Ѿ���¼
	};

	struct ClientEvent
	{
		uint32_t clientIdx;			// ��ǰ�¼�����ʱ�ͻ�������
		char userEvent[8];			// �¼�����
		FTPStatusCode statusCode;	// ״̬��
		char str1[MAX_PATH];		// �﷨��ֵĵ�һ���ַ���
		char str2[MAX_PATH];		// �﷨��ֵĵڶ����ַ���
	};
public:
	FTPServer();
	virtual ~FTPServer();

	// IServer�ӿ�ʵ��
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
	// �﷨��������
	void Parse(char * buffer, uint32_t bufferCount);
	bool ParsePath(std::istringstream& iss, int EventStr);
	bool ParseRest(std::istringstream& iss);
	std::string PathAppend(std::string curr, std::string extend);
	// ͨ�ù���
	void ShowHostIP();
	char* ShowServerTime(char * outBuffer, uint32_t bufferCount);
	void WriteLog();
	void SendCurrDirectories();
	void SendFile(SOCKET s, std::string file);
	void RecvFile(SOCKET s, std::string file);
	// ��ѭ������
	void UserCtrlConnect();
	void UserDataConnect();
	void UserSendCtrlMessage();
	void UserSendDataMessage();
	
	

private:
	// ���Ʋ���
	SOCKET								msCtrl;				// ���������Ƽ����׽���
	WSAEVENT							mhCtrl;				// WSA�����������¼�
	std::vector<WSAEVENT>				mhClientCtrls;		// �ͻ��˿����¼�
	std::vector<ClientInfo>				mClientCtrlInfos;	// �ͻ��˿�����Ϣ
	
	
	

	// ���䲿��
	SOCKET								msData;				// ���������ݼ����׽���
	WSAEVENT							mhData;				// WSA�����������¼�
	std::vector<WSAEVENT>				mhClientDatas;		// �ͻ��������¼�
	std::vector<ClientInfo>				mClientDataInfos;	// �ͻ���������Ϣ
	std::vector<std::thread>			mDataThreads;		// ���ݽ���

	std::ofstream						mLog;				// ��־�ļ���

	std::vector<std::unique_ptr<void, VoidDeleter>>	mUserInfos;	// �ͻ���Ϣ
	uint32_t							mUserBlockSize;			// �����û���ռ���ڴ��С

	ClientEvent							mClientEvent;		// ��ǰ�ͻ����¼�

	uint16_t							mPortControl;		// ���ƶ˿�
	uint16_t							mPortData;			// ���ݶ˿�


	std::experimental::filesystem::path mRootPath;			// �ɷ��ʸ�Ŀ¼
	std::experimental::filesystem::path mCurrPath;			// ��ǰĿ¼

};

bool FTPCreateServer(IServer** pOut);

