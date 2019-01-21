#pragma once
#include <WS2tcpip.h>
#include <ctime>
#include <filesystem>
#include "FTPHelper.h"



struct IServer
{
	virtual void Run() = 0;
};

bool FTPCreateServer(IServer** pOut);
bool FTPCloseServer();

struct IClient
{
	virtual void Run() = 0;
};

bool FTPCreateClient(IClient** pOut);
bool FTPCloseClient();