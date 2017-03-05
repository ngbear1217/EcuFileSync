#pragma once
#include "winsock2.h"
#include <iostream>

#pragma comment (lib, "ws2_32.lib")
#define NETWORK_PORT 16921
using namespace std;

class CNetComm
{
public:
	CNetComm(void);
	~CNetComm(void);
	SOCKET g_hsoListen;

	void Close();
	bool InitNetwork(int nPort);
	SOCKET GetListenSocket(int nPort, int nBacklog = SOMAXCONN);
};

