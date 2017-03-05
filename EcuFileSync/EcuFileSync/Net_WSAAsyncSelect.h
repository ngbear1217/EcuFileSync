#pragma once
#include "netcomm.h"

#include <set>

#define WM_ASYNC_SOCKET	WM_USER + 500 // 소켓 상태 변경 통지를 받기 위한 유저 정의 메세지 

typedef set<SOCKET> SOCK_SET; // 자식 소켓을 관리하기 위한 STL set 템플릿 컨테이너 사용

class CNet_WSAAsyncSelect:
	public CNetComm
{
public:
	CNet_WSAAsyncSelect(void);
	~CNet_WSAAsyncSelect(void);


	WNDCLASS wcls; //윈도우 메세지 전달 방식을 사용하기 위해 윈도우 클래스 선언
	HWND g_hMsgWnd;
	
};

LRESULT CALLBACK WndSockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
