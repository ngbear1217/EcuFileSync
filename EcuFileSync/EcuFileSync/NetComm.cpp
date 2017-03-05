#include "StdAfx.h"
#include "NetComm.h"



CNetComm::CNetComm(void)
{
	
}


CNetComm::~CNetComm(void)
{
	Close();
}
void CNetComm::Close(){

	if (g_hsoListen != INVALID_SOCKET){
		closesocket(g_hsoListen);
	}
	if (g_hsoListen == INVALID_SOCKET)
	{
		WSACleanup();
	}
}
bool CNetComm::InitNetwork(int nPort)
{
	WSADATA wsd;
	int nErrCode = WSAStartup(MAKEWORD(2,2), &wsd);
	if(nErrCode){
		cout << "WSAStartup failed with error : "<< nErrCode << endl;
	}

	g_hsoListen = GetListenSocket(nPort, 10);
	if(g_hsoListen == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}
	cout << " >>>>>>>>>>>>>>>> waiting for client's connection "<< endl;
	return true;
	
}
SOCKET CNetComm::GetListenSocket(int nPort, int nBacklog)
{
	//府郊 家南 积己
	SOCKET hsoListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(hsoListen == INVALID_SOCKET){
		cout << "socket failed, code : "<<WSAGetLastError() << endl;
		return INVALID_SOCKET;
	}

	// 官牢靛 林家 瘤沥
	SOCKADDR_IN sa;
	memset(&sa, 0, sizeof(SOCKADDR_IN));
	sa.sin_family		= AF_INET;
	sa.sin_port = htons(nPort);
	sa.sin_addr.s_addr	= htonl(INADDR_ANY);

	//家南 官牢爹
	int lSockRet = bind(hsoListen, (PSOCKADDR)&sa, sizeof(SOCKADDR_IN));
	if(lSockRet == SOCKET_ERROR){
		cout << "bind failed, code : "<<WSAGetLastError() << endl;
		closesocket(hsoListen);
		return INVALID_SOCKET;
	}

	//府郊
	lSockRet = listen(hsoListen, nBacklog);
	if(lSockRet == SOCKET_ERROR){
		cout << "listen failed, code : "<<WSAGetLastError() << endl;
		closesocket(hsoListen);
		return INVALID_SOCKET;
	}
	return hsoListen;
}


