#include "StdAfx.h"
#include "Net_WSAAsyncSelect.h"


CNet_WSAAsyncSelect::CNet_WSAAsyncSelect(void)
{
	

	/****************** ������ Ŭ���� ��� *******************/
	memset(&wcls, 0, sizeof(wcls));
	wcls.lpfnWndProc = WndSockProc; // �ݹ� �Լ��� ����
	wcls.hInstance = GetModuleHandle(NULL);
	wcls.lpszClassName = _T("WSAAyncSelect");
	if (!RegisterClass(&wcls))
	{
		cout << "send failed, code : " << GetLastError() << endl;
		return;
	}
	/**********************************************************************/
	/********************** ������ ������ ���� ******************************/
	g_hMsgWnd = NULL;
	SOCK_SET socks;

	HWND hWnd = CreateWindowEx
		(
		0, wcls.lpszClassName, NULL, 0, 0, 0, 0, 0,
		HWND_MESSAGE, NULL, wcls.hInstance,
		&socks // ���������� ����� ���ϰ� �ڽ� ���� ���� ��Ʈ ����Ʈ ����, WndSockProc�� lparam���� �Ѿ.
		);
	if (!hWnd)
	{
		cout << "send failed, code : " << GetLastError() << endl;
		return;
	}
	g_hMsgWnd = hWnd;
	cout << " ==> Creating hidden window success!!!" << endl;
	/**********************************************************************/
	/********************** Listen ���� ���� ******************************/
	if (!InitNetwork(NETWORK_PORT)){
		cout << "Socket Open failed, code : " << GetLastError() << endl;
		return;
	}
	WSAAsyncSelect(g_hsoListen, hWnd, WM_ASYNC_SOCKET, FD_ACCEPT); // Listen socket�� �����ϱ� ���� g_hsoListen�� hWnd�� �ѱ�, ���Ӹ� �����ϸ������ FD_ACCEPT ����
	cout << " ==> Waiting for client's connection......" << endl;

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (g_hsoListen != INVALID_SOCKET)
		closesocket(g_hsoListen);
	for (SOCK_SET::iterator it = socks.begin(); it != socks.end(); it++)
		closesocket(*it);
	cout << "Listen socket closed, program terminates..." << endl;
	Close();
}


CNet_WSAAsyncSelect::~CNet_WSAAsyncSelect(void)
{
	if (g_hMsgWnd != NULL)
		PostMessage(g_hMsgWnd, WM_DESTROY, 0, 0);
	//return TRUE;
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
		cout << " ==> Client " << sock << " WM_DESTROY..." << endl;
		PostQuitMessage(0); //�޼��� 
	}
	return 0;

	case WM_ASYNC_SOCKET: 
	{
		LONG lErrCode = WSAGETSELECTERROR(lParam);
		if (lErrCode != 0)	//WSAECONNABORTED
		{
			SOCKET sock = (SOCKET)wParam;
			cout << "~~~ socket " << sock << " failed: " << lErrCode << endl;
			closesocket(sock);
			s_pSocks->erase(sock);
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
				cout << "accept failed, code : " << WSAGetLastError() << endl;
				break;
			}

			WSAAsyncSelect(sock, hWnd, WM_ASYNC_SOCKET, FD_READ | FD_CLOSE);
			s_pSocks->insert(sock);
			cout << " ==> New client " << sock << " connected" << endl;
		}
		break;

		//case FD_READ:
		//{
		//	SOCKET sock = (SOCKET)wParam;
		//	char szBuff[512];
		//	int lSockRet = recv(sock, szBuff, sizeof(szBuff), 0);
		//	szBuff[lSockRet] = 0;
		//	cout << " *** Client(" << sock << ") sent : " << szBuff << endl;

		//	lSockRet = send(sock, szBuff, lSockRet, 0);
		//	if (lSockRet == SOCKET_ERROR)
		//		cout << "send failed, code : " << WSAGetLastError() << endl;
		//}
		//break;

		case FD_READ:
		{
			SOCKET sock = (SOCKET)wParam;
			char szBuff[512];
			int lSockRet = recv(sock, szBuff, sizeof(szBuff), 0);
			if (lSockRet <= 0)
				break;

			cout << " *** Client(" << sock << ") sent " << lSockRet << " bytes." << endl;
			for (SOCK_SET::iterator it = s_pSocks->begin(); it != s_pSocks->end(); it++)
			{
				SOCKET soOther = *it;
				if (soOther == sock){
					lSockRet = send(soOther, szBuff, lSockRet, 0);
					continue;
				}
									
				if (lSockRet == SOCKET_ERROR)
					cout << "send to client " << soOther << " failed, code : " << WSAGetLastError() << endl;
			}
		}
		break;

		case FD_CLOSE:
		{
			SOCKET sock = (SOCKET)wParam;
			closesocket(sock);
			s_pSocks->erase(sock);
			cout << " ==> Client " << sock << " disconnected..." << endl;
		}
		break;
		}
	}
	return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}