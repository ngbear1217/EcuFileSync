
// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

// MFC�� ���� �κа� ���� ������ ��� �޽����� ���� ����⸦ �����մϴ�.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.

#include "logWriter.h"

#define INI_PATH				_T("\\FLASH_SETTING.ini")

#ifndef WIN64
//#import "C:\Program Files\Common Files\System\ado\msado60.tlb"  rename("EOF", "EndOfFile") 
#import "C:\Program Files\Common Files\System\ado\msado60_Backcompat.tlb" rename("EOF", "EndOfFile")
#else
#import "C:\Program Files (x86)\Common Files\System\ado\msado60_Backcompat_i386.tlb" no_namespace rename("EOF", "EndOfFile") 
#endif






#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC�� ���� �� ��Ʈ�� ���� ����

#define USING_THREAD 1
#define CHKMEM_GetFileInfo 1
#define SEND_FILEINFO_TO_DEVICE 1
#define CHKMEM_GetFileInfo_sub2 1
#define USING_ADO15 1







