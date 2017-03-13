// WSASyncSelectDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EcuFileSync.h"
#include "WSASyncSelectDlg.h"
#include "afxdialogex.h"


// CWSASyncSelectDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CWSASyncSelectDlg, CDialogEx)

CWSASyncSelectDlg::CWSASyncSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWSASyncSelectDlg::IDD, pParent)
{

}

CWSASyncSelectDlg::~CWSASyncSelectDlg()
{
	if (cNetComm.g_hsoListen != INVALID_SOCKET)
		closesocket(cNetComm.g_hsoListen);
	for (SOCKET_SET::iterator it = m_sSocks.begin(); it != m_sSocks.end(); it++)
		closesocket(*it);
	U_Log("COMMON", "[%-40s]Listen socket closed, program terminates...", __FUNCTION__);
	cNetComm.Close();
}

void CWSASyncSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWSASyncSelectDlg, CDialogEx)
	ON_MESSAGE(WM_SOCK_NOTI, &CWSASyncSelectDlg::OnSockNoti)
END_MESSAGE_MAP()


// CWSASyncSelectDlg 메시지 처리기입니다.

bool CWSASyncSelectDlg::Init(int nPort)
{
	//CWnd hwnd = ;
	/********************** Listen 소켓 생성 ******************************/
	if (!cNetComm.InitNetwork(nPort)){
		U_Log("COMMON", "[%-40s]Socket Open failed, code : %d", __FUNCTION__, GetLastError());
		return false;
	}
	WSAAsyncSelect(cNetComm.g_hsoListen, GetSafeHwnd(), WM_SOCK_NOTI, FD_ACCEPT); // Listen socket을 감시하기 위해 g_hsoListen과 hWnd를 넘김, 접속만 감시하면됨으로 FD_ACCEPT 선언
	//cout << " ==> Waiting for client's connection......" << endl;
	U_Log("COMMON", "[%-40s]Waiting for client's connection......", __FUNCTION__);

	/*
	if (g_hsoListen != INVALID_SOCKET)
		closesocket(g_hsoListen);
	for (SOCK_SET::iterator it = socks.begin(); it != socks.end(); it++)
		closesocket(*it);
	U_Log("COMMON", "[%-40s]Listen socket closed, program terminates...", __FUNCTION__);
	Close();*/
	return true;
}

//
//// // 윈도우 쓰될때 호출
//LRESULT CALLBACK WndSockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	static SOCK_SET* s_pSocks = NULL;
//
//	switch (uMsg)
//	{
//	case WM_CREATE:
//	{
//		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
//		s_pSocks = (SOCK_SET*)pCS->lpCreateParams;
//	}
//	return 0;
//
//	case WM_DESTROY:
//	{
//		SOCKET sock = (SOCKET)wParam;
//		//	cout << " ==> Client " << sock << " WM_DESTROY..." << endl;
//		U_Log("COMMON", "[%-40s]Client %d WM_DESTROY...", __FUNCTION__, sock);
//		PostQuitMessage(0); //메세지 
//	}
//	return 0;
//
//	case WM_ASYNC_SOCKET:
//	{
//		LONG lErrCode = WSAGETSELECTERROR(lParam);
//		if (lErrCode != 0)	//WSAECONNABORTED
//		{
//			SOCKET sock = (SOCKET)wParam;
//			U_Log("COMMON", "[%-40s]socket %d failed:%d ", __FUNCTION__, sock, lErrCode);
//			closesocket(sock);
//
//			//s_pSocks->erase(sock);
//			return 0;
//		}
//
//		switch (WSAGETSELECTEVENT(lParam))
//		{
//		case FD_ACCEPT:
//		{
//			SOCKET hsoListen = (SOCKET)wParam;
//			SOCKET sock = accept(hsoListen, NULL, NULL);
//			if (sock == INVALID_SOCKET)
//			{
//				U_Log("COMMON", "[%-40s]accept failed, code : %d", __FUNCTION__, WSAGetLastError());
//				break;
//			}
//
//			WSAAsyncSelect(sock, hWnd, WM_ASYNC_SOCKET, FD_READ | FD_CLOSE);
//			s_pSocks->insert(sock);
//			U_Log("COMMON", "[%-40s]New client : %d connected ", __FUNCTION__, sock);
//		}
//		break;
//
//		case FD_READ:
//		{
//			SOCKET sock = (SOCKET)wParam;
//			char szBuff[512];
//			int lSockRet = recv(sock, szBuff, sizeof(szBuff), 0);
//			if (lSockRet <= 0)
//				break;
//
//
//			U_Log("COMMON", "[%-40s] *** Client(%d) sent (%d) bytes.", __FUNCTION__, sock, lSockRet);
//			for (SOCK_SET::iterator it = s_pSocks->begin(); it != s_pSocks->end(); it++)
//			{
//				SOCKET soOther = *it;
//				if (soOther == sock){
//					lSockRet = send(soOther, szBuff, lSockRet, 0);
//					continue;
//				}
//
//				if (lSockRet == SOCKET_ERROR)
//					U_Log("COMMON", "[%-40s] send to client (%d)failed, code : ", __FUNCTION__, soOther, WSAGetLastError());
//			}
//		}
//		break;
//
//		case FD_CLOSE:
//		{
//			SOCKET sock = (SOCKET)wParam;
//			closesocket(sock);
//			s_pSocks->erase(sock);
//			cout << " ==> Client " << sock << " disconnected..." << endl;
//			U_Log("COMMON", "[%-40s] ==> Client (%d) disconnected...", __FUNCTION__, sock);
//		}
//		break;
//		}
//	}
//	return 0;
//	}
//
//	return DefWindowProc(hWnd, uMsg, wParam, lParam);
//}

afx_msg LRESULT CWSASyncSelectDlg::OnSockNoti(WPARAM wParam, LPARAM lParam)
{
	SOCKET	sock = (SOCKET)wParam;
	LONG	lErrCode = WSAGETSELECTERROR(lParam);
	WORD	wFDFlags = WSAGETSELECTEVENT(lParam);

	if (lErrCode != 0)
	{
		U_Log("COMMON", "[%-40s]socket %d failed:%d ", __FUNCTION__, sock, lErrCode);
		closesocket(sock);
	
		//GetDlgItem(IDC_EDIT_CHAT)->EnableWindow(FALSE);
		//GetDlgItem(IDC_EDIT_INPUT)->EnableWindow(FALSE);
		//GetDlgItem(IDOK)->SetWindowText(_T("접속"));
		return 0;
	}

	switch (wFDFlags)
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

		WSAAsyncSelect(sock, this->GetSafeHwnd(), WM_SOCK_NOTI, FD_READ | FD_CLOSE);
		m_sSocks.insert(sock);
		U_Log("COMMON", "[%-40s]New client : %d connected ", __FUNCTION__, sock);
	}
	break;

	case FD_READ:
	{
		TCHAR szBuff[65536];
		int lSockRet = recv(sock, (PCHAR)szBuff, 65536 * sizeof(TCHAR), 0);
		szBuff[lSockRet / sizeof(TCHAR)] = 0;
		_tcscat_s(szBuff, _T("\xd\xa"));

		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHAT);
		int nInsPos = pEdit->GetWindowTextLength();
		pEdit->SetSel(nInsPos, nInsPos);
		pEdit->ReplaceSel(szBuff);
	}
	break;

	case FD_CLOSE:
	{
		SOCKET sock = (SOCKET)wParam;
		closesocket(sock);
		m_sSocks.erase(sock);
		U_Log("COMMON", "[%-40s] ==> Client (%d) disconnected...", __FUNCTION__, sock);
	}
	break;
	}
	return 0;
}

void CWSASyncSelectDlg::PostNcDestroy(){
	delete this;
}