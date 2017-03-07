
// EcuFileSyncDlg.h : ��� ����
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
// CEcuFileSyncDlg ��ȭ ����
class CEcuFileSyncDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CEcuFileSyncDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ECUFILESYNC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;
	// ������ �޽��� �� �Լ�
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