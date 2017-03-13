#pragma once
#include "stdafx.h"
#include "NetComm.h"
#include <set>
#include "resource.h"
#define WM_ASYNC_SOCKET	WM_USER + 500
#define WM_SOCK_NOTI	WM_USER + 501

// CWSASyncSelectDlg 대화 상자입니다.
using namespace std;
typedef set<SOCKET> SOCKET_SET;
class CWSASyncSelectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWSASyncSelectDlg)

public:
	CWSASyncSelectDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CWSASyncSelectDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_WSASyncSelet };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	CNetComm cNetComm;
	SOCKET_SET m_sSocks;
	bool Init(int nPort);
	void PostNcDestroy();
protected:
	afx_msg LRESULT OnSockNoti(WPARAM wParam, LPARAM lParam);
};
