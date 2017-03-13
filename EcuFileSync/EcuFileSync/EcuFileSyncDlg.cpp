
// EcuFileSyncDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "EcuFileSync.h"
#include "EcuFileSyncDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_TRAY_NOTIFICATION WM_APP + 1

CEcuFileSyncDlg::CEcuFileSyncDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEcuFileSyncDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEcuFileSyncDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PORT, m_Edit_Port);
	DDX_Control(pDX, IDC_EDIT_MAINVIEW, m_EDIT_MAIN);
	DDX_Control(pDX, IDC_CMB_BINLIST, mCMB_FILELIST);
	DDX_Control(pDX, IDC_BTN_SOCK_OPEN, m_btn_open);
	DDX_Control(pDX, IDC_BTN_SOCK_CLOSE, m_btn_close);
	DDX_Control(pDX, IDC_EDT_DIR, m_Edit_FileList);
}

BEGIN_MESSAGE_MAP(CEcuFileSyncDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SOCK_OPEN, &CEcuFileSyncDlg::OnBnClickedBtnSockOpen)
	ON_BN_CLICKED(IDC_BTN_SOCK_CLOSE, &CEcuFileSyncDlg::OnBnClickedSockClose)

	ON_MESSAGE(MSG_CONNECT, OnClientConnected)
	ON_MESSAGE(MSG_REMOVECLIENT, OnClientDisConnected)
	//ON_MESSAGE(MSG_FILEUP_ERR, OnFileUpErr)
	ON_MESSAGE(MSG_FILEUP_CHK, OnReceiveFileChk)
//	ON_MESSAGE(MSG_SENDFILEINFO, OnSendFileInfo) //step1
	ON_MESSAGE(MSG_RECVFILEINFO, OnReceiveFileInfo) //step2
	
	ON_MESSAGE(MSG_SUBMSG, OnShowMessage)
	

	ON_BN_CLICKED(IDC_CHK_ALL, &CEcuFileSyncDlg::OnBnClickedChkAll)
	ON_BN_CLICKED(IDC_BTN_FOLDER, &CEcuFileSyncDlg::OnBnClickedBtnFolder)
	ON_BN_CLICKED(IDC_BTN_SEND_ALL, &CEcuFileSyncDlg::OnBnClickedBtnSendAll)
	ON_BN_CLICKED(IDC_BTN_SENDFILE, &CEcuFileSyncDlg::OnBnClickedBtnSendfile)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CEcuFileSyncDlg 메시지 처리기

BOOL CEcuFileSyncDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	m_hbm_LEDOff = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LED_OFF));
	m_hbm_LEDOn = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LED_ON));


	char strCurPath[256] = { 0, };
	GetCurrentDirectory(sizeof(strCurPath), strCurPath);
	m_strINIPath.Format("%s\\%s", strCurPath, INI_PATH);

	CString strMainPath;
	char cbuf[256] = { 0, };
	GetPrivateProfileString("CONFIGURATION_INFO", "ECUFILE_PATH", "", cbuf, sizeof(cbuf), m_strINIPath);
	strMainPath.Format("%s", cbuf);
	m_strDirPath.Format("%s", strMainPath);

	//DWORD dwThreadId1 = 0;
	//HANDLE hThread2 = CreateThread(NULL, 0, TimeCheckThread, (LPVOID)this, 0, &dwThreadId1);
	//CloseHandle(hThread2);

	//TrayStateSetup(NIM_ADD, "VCI_SERVER");// 트레이에 등록
	//ShowWindow(SW_SHOWMINIMIZED);

	int nport = 16921;
	m_pNetServerDlg = NULL;

#ifdef USING_WSASync

	//m_NetCtrl = new CNet_WSAAsyncSelect();
	//m_NetCtrl->Init(nport);
#else

	m_NetCtrl.Init(nport);
	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, ServerStart, &m_NetCtrl, 0, &dwThreadId);
	CloseHandle(hThread);
#endif

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}



// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CEcuFileSyncDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CEcuFileSyncDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//////////////////////////////////// function ///////////////////////////////////


void CEcuFileSyncDlg::SetViewObject(ClientObject* pClientObj){

	int nVciNo = 0;
	bool bBoadNewYN = true;
	for (int i = VIEW_NO::BOARD1; i <= VIEW_NO::BOARD10; i++){
		
#ifdef USING_HASHMAP
		hash_map<int, ClientObject*>::iterator Iter;
		for (Iter = m_ClientMng.m_Objects.begin(); Iter != m_ClientMng.m_Objects.end(); ++Iter){
			if (((Iter->second))->mViewNo == i){
				bBoadNewYN = false;
				break;
			}
			bBoadNewYN = true;
		}
#else
		CLIENT_SET::iterator Iter;
		ClientObject* pClientObj;
		for (Iter = m_pClientMng->g_sClients.begin(); Iter != m_pClientMng->g_sClients.end(); ++Iter){
			pClientObj = *Iter;
			if (pClientObj->mViewNo == i){
				bBoadNewYN = false;
				break;
			}
			bBoadNewYN = true;
		}
#endif
		if (bBoadNewYN){
			nVciNo = i;
			break;
		}
		else{
			continue;
		}
	}
	pClientObj->SetStatic(nVciNo, bBoadNewYN);
}
void CEcuFileSyncDlg::SetLedImage(UINT nCtrlID, bool bOnOff){

	((CStatic*)GetDlgItem(nCtrlID))->SetBitmap(bOnOff ? m_hbm_LEDOn : m_hbm_LEDOff);
	((CStatic*)GetDlgItem(nCtrlID))->Invalidate();

}



void CEcuFileSyncDlg::TrayStateSetup(int parm_command, const char *parm_tip_string)
{
	NOTIFYICONDATA taskbar_notify_data;
	strcpy(taskbar_notify_data.szTip, parm_tip_string);
	switch (parm_command){
		// 트레이에 등록
	case NIM_ADD:
		taskbar_notify_data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		break;
		// 트레이에서 해제
	case NIM_DELETE:
		taskbar_notify_data.uFlags = 0;
		break;
		// 이미 등록된 정보를 변경
	case NIM_MODIFY:
		taskbar_notify_data.uFlags = NIF_TIP | NIF_ICON;
		break;
	}
	taskbar_notify_data.uID = (UINT)IDR_MAINFRAME;
	taskbar_notify_data.cbSize = sizeof(NOTIFYICONDATA);
	taskbar_notify_data.hWnd = this->m_hWnd;
	// 트레이 아이콘에서 발생되는 상태에 대한 정보를 건네 받을 메시지 지정
	taskbar_notify_data.uCallbackMessage = WM_TRAY_NOTIFICATION;
	// IDI_ICON1 : 리소스탭의 Icon 항목에 트레이에 등록됐을 때 나타낼 아이콘의 아이디
	taskbar_notify_data.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// 트레이에 등록할 정보(NOTIFYICONDATA)를 시스템에 전달하여 트레이에 등록한다.
	Shell_NotifyIcon(parm_command, &taskbar_notify_data);
}

void CEcuFileSyncDlg::GetFileList(list<CString>* listNm, CString filePath)
{
	//CString strFilePath;
	//strFilePath.Format(_T("%s\\*.*"), filePath);
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(filePath, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				listNm->push_back(fd.cFileName);

			}
		} while (::FindNextFile(hFind, &fd));
	}

	FindClose(hFind);

}

//파일 전송
bool CEcuFileSyncDlg::FileUpload(ClientObject* pClientObj, CString strFileName)
{
	int nFileCnt = 0;

	CString strFromFileName, strVCIFilePath, strServerFilePath;//, strIP;
	CString strVCIDirPath;

	strServerFilePath.Format("%s\\%s", m_strDirPath, strFileName);

	if (pClientObj != NULL)
	{
		pClientObj->SendPacketForFileInfo(strServerFilePath);
	}
	return true;
}
//Ecu정보 조회 masterDB 연결
//////////////////////////////////// msg event function ///////////////////////////////////
void CEcuFileSyncDlg::OnBnClickedBtnSockOpen()
{
		DWORD dwThreadId = 0;
	
		//nc.m_nPort = nPort;
#ifdef USING_WSASync
		/*if (m_NetCtrl != NULL)
			m_NetCtrl = new CNet_WSAAsyncSelect();
		HANDLE hThread = CreateThread(NULL, 0, ServerStart, &m_NetCtrl, 0, &dwThreadId);
		CloseHandle(hThread);*/
		if (m_pNetServerDlg == NULL){
			m_pNetServerDlg = new CWSASyncSelectDlg();
			m_pNetServerDlg->Create(CWSASyncSelectDlg::IDD, this);
			m_pNetServerDlg->ShowWindow(SW_SHOW);
			m_pNetServerDlg->Init(16921);
		}
		else{
			m_pNetServerDlg->SetFocus();
		}
		

		
		//m_EDIT_MAIN.SetWindowText("Socket Open");
		//m_btn_open.EnableWindow(false);
		//m_btn_close.EnableWindow(true);
		//	GetDlgItem(IDC_BTN_SOCK_START)->EnableWindow(false);
		//	GetDlgItem(IDC_BTN_SOCK_CLOSE)->EnableWindow(true);
#else

	int nPort = 16921;
	if (nPort > 0){
		if (m_NetCtrl.Init(nPort))
		{

			m_EDIT_MAIN.SetWindowText("Socket Open");
			m_btn_open.EnableWindow(false);
			m_btn_close.EnableWindow(true);
			//	GetDlgItem(IDC_BTN_SOCK_START)->EnableWindow(false);
			//	GetDlgItem(IDC_BTN_SOCK_CLOSE)->EnableWindow(true);

			HANDLE hThread = CreateThread(NULL, 0, ServerStart, &m_NetCtrl, 0, &dwThreadId);
			CloseHandle(hThread);
		}
	}
	else{
		AfxMessageBox("Port 입력하시오.", 0);
	}
#endif
	
}




void CEcuFileSyncDlg::OnBnClickedSockClose() // socket close
{
#ifdef USING_WSASync
	//if (m_NetCtrl != NULL){
	//	delete m_NetCtrl;
	//	m_NetCtrl = NULL;
	//}
	if (m_pNetServerDlg != NULL){
		m_pNetServerDlg->DestroyWindow();
		m_pNetServerDlg = NULL;
	}
#else
	m_NetCtrl.ServerClose();
#endif
	//m_btn_open.EnableWindow(true);
	//m_btn_close.EnableWindow(false);
	//GetDlgItem(IDC_BTN_SOCK_START)->EnableWindow(true);
	//GetDlgItem(IDC_BTN_SOCK_CLOSE)->EnableWindow(false);
}

LRESULT CEcuFileSyncDlg::OnClientConnected(WPARAM wParam, LPARAM lParam)
{
	ClientObject* pClientObj = (ClientObject*)lParam;

	if (pClientObj->m_nConnectType == CONNECT_TYPE::NEW_CONNECT)
	{
		SetViewObject(pClientObj);
		SetLedImage(pClientObj->unStaticLed, 1);
		SetDlgItemText(pClientObj->unView, "FileUpload Start");
		SetDlgItemText(pClientObj->unStaticIP, pClientObj->m_strIP);
#ifdef CHKMEM_GetFileInfo
		pClientObj->GetFileInfo();
#endif
	}
	return 0;

}

//Client Disconnect Event
LRESULT CEcuFileSyncDlg::OnClientDisConnected(WPARAM wParam, LPARAM lParam)
{

	return 0;
}


LRESULT CEcuFileSyncDlg::OnFileEmpty(WPARAM wParam, LPARAM lParam){
	ClientObject* pClientObj = (ClientObject*)lParam;

		U_SubLog("COMMON", pClientObj->m_cstrIP, "[%-40s]NO Update File ", __FUNCTION__);
		U_Log("COMMON", "[%-40s]FileUpload END : %s", __FUNCTION__, pClientObj->m_strIP);
		SetDlgItemText(pClientObj->unView, "NO Update File");
		
	return 0;
}


LRESULT CEcuFileSyncDlg::OnReceiveFileInfo(WPARAM wParam, LPARAM lParam){
	ClientObject* pClientObj = (ClientObject*)lParam;

		U_SubLog("COMMON", pClientObj->m_cstrIP, "[%-40s]FileStreamStart , FileName : %s ", __FUNCTION__, pClientObj->m_strServerFileName);
		//SetDlgItemText(pClientObj->unView, "SendFileStream Start");

		pClientObj->SendPacketForFileStream();
	
	return 0;
}
LRESULT CEcuFileSyncDlg::OnShowMessage(WPARAM wParam, LPARAM lParam){
	ClientObject* pClientObj = (ClientObject*)lParam;

	SetDlgItemText(pClientObj->unView, pClientObj->m_strMessage);
	UpdateData(false);
	return 0;
}


LRESULT CEcuFileSyncDlg::OnReceiveFileChk(WPARAM wParam, LPARAM lParam){
	ClientObject* pClientObj = (ClientObject*)lParam;

	//pClientObj->GetFileInfo();
	return 0;
}


void CEcuFileSyncDlg::OnBnClickedChkAll()
{
	if (IsDlgButtonChecked(IDC_CHK_ALL))
	{
		CheckDlgButton(IDC_CHK_VCI1, TRUE);
		CheckDlgButton(IDC_CHK_VCI2, TRUE);
		CheckDlgButton(IDC_CHK_VCI3, TRUE);
		CheckDlgButton(IDC_CHK_VCI4, TRUE);
		CheckDlgButton(IDC_CHK_VCI5, TRUE);
		CheckDlgButton(IDC_CHK_VCI6, TRUE);
		CheckDlgButton(IDC_CHK_VCI7, TRUE);
		CheckDlgButton(IDC_CHK_VCI8, TRUE);
		CheckDlgButton(IDC_CHK_VCI9, TRUE);
		CheckDlgButton(IDC_CHK_VCI10, TRUE);
	}
	else{
		CheckDlgButton(IDC_CHK_VCI1, FALSE);
		CheckDlgButton(IDC_CHK_VCI2, FALSE);
		CheckDlgButton(IDC_CHK_VCI3, FALSE);
		CheckDlgButton(IDC_CHK_VCI4, FALSE);
		CheckDlgButton(IDC_CHK_VCI5, FALSE);
		CheckDlgButton(IDC_CHK_VCI6, FALSE);
		CheckDlgButton(IDC_CHK_VCI7, FALSE);
		CheckDlgButton(IDC_CHK_VCI8, FALSE);
		CheckDlgButton(IDC_CHK_VCI9, FALSE);
		CheckDlgButton(IDC_CHK_VCI10, FALSE);
	}
}



void CEcuFileSyncDlg::OnBnClickedBtnSendfile()
{
	//check chkbox
	
	
	CString strFileName;
	mCMB_FILELIST.GetWindowText(strFileName);
#ifdef USING_HASHMAP
	ClientObject* pClientObj;
	hash_map<int, ClientObject*>::iterator Iter;
	for (Iter = m_ClientMng.m_Objects.begin(); Iter != m_ClientMng.m_Objects.end(); ++Iter){

		pClientObj = (Iter->second);
		if (IsDlgButtonChecked(pClientObj->unChkBox))
		{
			FileUpload(pClientObj, strFileName);
		}
	}
#else
	ClientObject* pClientObj;
	CLIENT_SET::iterator Iter;
	for (Iter = m_pClientMng->g_sClients.begin(); Iter != m_pClientMng->g_sClients.end(); ++Iter){
		pClientObj = *Iter;
		if (IsDlgButtonChecked(pClientObj->unChkBox))
		{
			FileUpload(pClientObj, strFileName);
		}
	}
#endif

}


void CEcuFileSyncDlg::OnBnClickedBtnSendAll()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}



////////////////////////////////////// Thread Function ///////////////////////////////////////////////////
DWORD WINAPI ServerStart(PVOID param)
{
#ifdef USING_WSASync
	CNet_WSAAsyncSelect* pNc = (CNet_WSAAsyncSelect*)param;
	pNc->Init(16921);
#else
	NetworkController* pNc = (NetworkController*)param;
	pNc->AcceptProcess();

#endif
	return 0;
}




void CEcuFileSyncDlg::OnBnClickedBtnFolder()
{
	BROWSEINFO BrInfo;
	CString strFolderPath;
	TCHAR pszPathname[100];
	ITEMIDLIST *pidIBrowse;
	BrInfo.hwndOwner = GetSafeHwnd();
	BrInfo.pidlRoot = NULL;

	memset(pszPathname, 0, sizeof(pszPathname));
	memset(&BrInfo, 0, sizeof(BrInfo));
	BrInfo.pszDisplayName = pszPathname;
	BrInfo.lpszTitle = _T("Select Directory");
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	//BrInfo.lpfn = BrowseCallbackProc; // event function

	//CString strInitPath = _T("D:\\");
	//BrInfo.lParam = (LPARAM)strInitPath.GetBuffer();

	pidIBrowse = ::SHBrowseForFolder(&BrInfo);
	if (pidIBrowse != NULL){
		SHGetPathFromIDList(pidIBrowse, pszPathname);
	}

	m_strDirPath.Format(_T("%s"), pszPathname);
	m_Edit_FileList.SetWindowText(m_strDirPath);


	/////////////// File List 생성 ////////
	list<CString> FileList;
	CString strFilePath;

	strFilePath.Format("%s\\*.*", m_strDirPath);
	GetFileList(&FileList, strFilePath);

	mCMB_FILELIST.ResetContent();

	list< CString >::iterator iterEnd = FileList.end();
	list< CString >::iterator iterPos;
	for (iterPos = FileList.begin(); iterPos != iterEnd; ++iterPos)
	{
		//cout << "아이템 코드 : " << iterPos->ItemCd << endl;
		mCMB_FILELIST.AddString(*iterPos);
	}

	//iterEnd = IniFileList.end();//파일 List 조회

	GetDlgItem(IDC_BTN_SOCK_OPEN)->EnableWindow(true);


}

void CEcuFileSyncDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
#ifdef USING_WSASync
	if(m_NetCtrl != NULL){
		delete m_NetCtrl;
		m_NetCtrl = NULL;
	}
	
#else
	m_NetCtrl.ServerClose();
#endif
	CDialogEx::OnClose();
}


void CEcuFileSyncDlg::InitDB()
{

}


void CEcuFileSyncDlg::CloseDB()
{
}
