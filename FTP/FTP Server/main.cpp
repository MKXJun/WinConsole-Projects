#include "FTP.h"
#include "Console.h"
int main()
{
	// 初始化WS2_32.dll
	WSADATA wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		MessageBoxA(NULL, "Winsock初始化失败！", "错误", MB_OK);
		exit(0);
	}
	
	IServer* pServer;
	FTPCreateServer(&pServer);

	pServer->Run();


	FTPCloseServer();

	WSACleanup();
	system("PAUSE");
}