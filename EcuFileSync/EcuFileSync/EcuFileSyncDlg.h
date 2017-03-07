
// EcuFileSyncDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "ProtocolDef.h"
#include "ClientObject.h"
#include "ClientMng.h"
#include "WSASyncSelectDlg.h"
#include <list>
#include <hash_map>
#ifdef USING_WSASync
#include "Net_WSAAsyncSelect.h"
#else
#include "NetworkController.h"
#endif


using namespace std;
// CEcuFileSyncDlg 대화 상자
class CEcuFileSyncDlg : public CDialogEx
{
// 생성입니다.
public:
	CEcuFileSyncDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ECUFILESYNC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	//afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtnSockOpen();
	afx_msg void OnBnClickedSockClose();
	afx_msg void OnBnClickedChkAll();
	afx_msg void OnBnClickedBtnFolder();

	LRESULT OnClientConnected(WPARAM wParam, LPARAM lParam);
	LRESULT OnClientDisConnected(WPARAM wParam, LPARAM lParam);
	//LRESULT OnFileUpErr(WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendFileInfo(WPARAM wParam, LPARAM lParam);
	LRESULT OnReceiveFileInfo(WPARAM wParam, LPARAM lParam);
	
	LRESULT OnShowMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnReceiveFileChk(WPARAM wParam, LPARAM lParam);

	LRESULT OnFileEmpty(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
public:
	
	CWSASyncSelectDlg m_NetServerDlg;
	CEdit m_Edit_Port;
	CEdit m_EDIT_MAIN;
	HBITMAP m_hbm_LEDOn;
	HBITMAP m_hbm_LEDOff;
#ifdef USING_WSASync
	CNet_WSAAsyncSelect* m_NetCtrl;
#else
	NetworkController m_NetCtrl;
#endif
	
	CClientMng* m_pClientMng;

	bool m_bChkCloseThread;
	bool m_bChkClose;
	CString m_strINIPath;
	CString m_strDirPath;
	CString m_strUpdateFileFullPath;
	
	void SetViewObject(ClientObject* pClientObj);
	void SetLedImage(UINT nCtrlID, bool bOnOff);
	void TrayStateSetup(int parm_command, const char *parm_tip_string);
	void GetFileList(list<CString>* listNm, CString filePath);
	bool FileUpload(ClientObject* pClientObj, CString strFileName);
	

	CComboBox mCMB_FILELIST;
	CButton m_btn_open;
	CButton m_btn_close;
	
	CEdit m_Edit_FileList;
	afx_msg void OnBnClickedBtnSendAll();
	afx_msg void OnBnClickedBtnSendfile();
	afx_msg void OnClose();

public:
	
	void InitDB();
	void CloseDB();
};


DWORD WINAPI ServerStart(PVOID param);