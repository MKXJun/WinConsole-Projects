#include "FTP.h"
#include "Console.h"
int main()
{
	// ��ʼ��WS2_32.dll
	WSADATA wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		MessageBoxA(NULL, "Winsock��ʼ��ʧ�ܣ�", "����", MB_OK);
		exit(0);
	}
	
	IServer* pServer;
	FTPCreateServer(&pServer);

	pServer->Run();


	FTPCloseServer();

	WSACleanup();
	system("PAUSE");
}