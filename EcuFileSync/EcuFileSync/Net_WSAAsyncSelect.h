#pragma once
#include "netcomm.h"

#include <set>

#define WM_ASYNC_SOCKET	WM_USER + 500 // ���� ���� ���� ������ �ޱ� ���� ���� ���� �޼��� 

typedef set<SOCKET> SOCK_SET; // �ڽ� ������ �����ϱ� ���� STL set ���ø� �����̳� ���

class CNet_WSAAsyncSelect:
	public CNetComm
{
public:
	CNet_WSAAsyncSelect(void);
	~CNet_WSAAsyncSelect(void);


	WNDCLASS wcls; //������ �޼��� ���� ����� ����ϱ� ���� ������ Ŭ���� ����
	HWND g_hMsgWnd;
	
};

LRESULT CALLBACK WndSockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
