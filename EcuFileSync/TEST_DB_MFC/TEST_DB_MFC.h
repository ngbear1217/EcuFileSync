
// TEST_DB_MFC.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CTEST_DB_MFCApp:
// �� Ŭ������ ������ ���ؼ��� TEST_DB_MFC.cpp�� �����Ͻʽÿ�.
//

class CTEST_DB_MFCApp : public CWinApp
{
public:
	CTEST_DB_MFCApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CTEST_DB_MFCApp theApp;