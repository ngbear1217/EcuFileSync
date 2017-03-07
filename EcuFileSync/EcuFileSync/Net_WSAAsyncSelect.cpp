#include "StdAfx.h"
#include "Net_WSAAsyncSelect.h"
#include "WSASyncSelectDlg.h"
#include "ProtocolDef.h"


CNet_WSAAsyncSelect::CNet_WSAAsyncSelect(void)
{
	
}


CNet_WSAAsyncSelect::~CNet_WSAAsyncSelect(void)
{
	if (g_hMsgWnd != NULL)
		PostMessage(g_hMsgWnd, WM_DESTROY, 0, 0);
	//return TRUE;
}

bool CNet_WSAAsyncSelect::Init(int nPort)
{
	/****************** ������ Ŭ���� ��� *******************/
	memset(&wcls, 0, sizeof(wcls));
	wcls.lpfnWndProc = WndSockProc; // �ݹ� �Լ��� ����
	wcls.hInstance = GetModuleHandle(NULL);
	wcls.lpszClassName = _T("WSAAyncSelect");
	if (!RegisterClass(&wcls))
	{
		U_Log("COMMON", "[%-40s]send failed, code : %d", __FUNCTION__, GetLastError());
		return false;
	}
	/**********************************************************************/
	/********************** ������ ������ ���� ******************************/
	g_hMsgWnd = NULL;
	SOCK_SET socks;

	HWND hWnd = CreateWindowEx
		(
		0, wcls.lpszClassName, NULL, 100,100, 100, 100, 100,
		HWND_MESSAGE, NULL, wcls.hInstance,
		&socks // ���������� ����� ���ϰ� �ڽ� ���� ���� ��Ʈ ����Ʈ ����, WndSockProc�� lparam���� �Ѿ.
		);
	if (!hWnd)
	{
		U_Log("COMMON", "[%-40s]send failed, code : %d", __FUNCTION__, GetLastError());
		return false;
	}
	g_hMsgWnd = hWnd;
	U_Log("COMMON", "[%-40s]Creating hidden window success!!!", __FUNCTION__);
	/**********************************************************************/
	/********************** Listen ���� ���� ******************************/
	if (!InitNetwork(nPort)){
		U_Log("COMMON", "[%-40s]Socket Open failed, code : %d", __FUNCTION__, GetLastError());
		return false;
	}
	WSAAsyncSelect(g_hsoListen, hWnd, WM_ASYNC_SOCKET, FD_ACCEPT); // Listen socket�� �����ϱ� ���� g_hsoListen�� hWnd�� �ѱ�, ���Ӹ� �����ϸ������ FD_ACCEPT ����
	//cout << " ==> Waiting for client's connection......" << endl;
	U_Log("COMMON", "[%-40s]Waiting for client's connection......", __FUNCTION__);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (g_hsoListen != INVALID_SOCKET)
		closesocket(g_hsoListen);
	for (SOCK_SET::iterator it = socks.begin(); it != socks.end(); it++)
		closesocket(*it);
	U_Log("COMMON", "[%-40s]Listen socket closed, program terminates...", __FUNCTION__);
	Close();
	return true;
}


// // ������ ���ɶ� ȣ��
LRESULT CALLBACK WndSockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static SOCK_SET* s_pSocks = NULL;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		s_pSocks = (SOCK_SET*)pCS->lpCreateParams;
	}
	return 0;

	case WM_DESTROY:
	{
		SOCKET sock = (SOCKET)wParam;
	//	cout << " ==> Client " << sock << " WM_DESTROY..." << endl;
		U_Log("COMMON", "[%-40s]Client %d WM_DESTROY...", __FUNCTION__, sock);
		PostQuitMessage(0); //�޼��� 
	}
	return 0;

	case WM_ASYNC_SOCKET: 
	{
		LONG lErrCode = WSAGETSELECTERROR(lParam);
		if (lErrCode != 0)	//WSAECONNABORTED
		{
			SOCKET sock = (SOCKET)wParam;
			U_Log("COMMON", "[%-40s]socket %d failed:%d ", __FUNCTION__, sock, lErrCode);
			closesocket(sock);
			
			//s_pSocks->erase(sock);
			return 0;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
		{
			SOCKET hsoListen = (SOCKET)wParam;
			SOCKET sock = accept(hsoListen, NULL, NULL);
			if (sock == INVALID_SOCKET)
			{
				U_Log("COMMON", "[%-40s]accept failed, code : %d", __FUNCTION__, WSAGetLastError());
				break;
			}

			WSAAsyncSelect(sock, hWnd, WM_ASYNC_SOCKET, FD_READ | FD_CLOSE);
			s_pSocks->insert(sock);
			U_Log("COMMON", "[%-40s]New client : %d connected ", __FUNCTION__, sock);
		}
		break;

		case FD_READ:
		{
			SOCKET sock = (SOCKET)wParam;
			char szBuff[512];
			int lSockRet = recv(sock, szBuff, sizeof(szBuff), 0);
			if (lSockRet <= 0)
				break;
			
			
			U_Log("COMMON", "[%-40s] *** Client(%d) sent (%d) bytes.", __FUNCTION__, sock, lSockRet);
			for (SOCK_SET::iterator it = s_pSocks->begin(); it != s_pSocks->end(); it++)
			{
				SOCKET soOther = *it;
				if (soOther == sock){
					lSockRet = send(soOther, szBuff, lSockRet, 0);
					continue;
				}
									
				if (lSockRet == SOCKET_ERROR)
					U_Log("COMMON", "[%-40s] send to client (%d)failed, code : ", __FUNCTION__, soOther, WSAGetLastError());
			}
		}
		break;

		case FD_CLOSE:
		{
			SOCKET sock = (SOCKET)wParam;
			closesocket(sock);
			s_pSocks->erase(sock);
			cout << " ==> Client " << sock << " disconnected..." << endl;
			U_Log("COMMON", "[%-40s] ==> Client (%d) disconnected...", __FUNCTION__, sock);
		}
		break;
		}
	}
	return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}