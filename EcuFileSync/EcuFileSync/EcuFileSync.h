
// EcuFileSync.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CEcuFileSyncApp:
// �� Ŭ������ ������ ���ؼ��� EcuFileSync.cpp�� �����Ͻʽÿ�.
//

class CEcuFileSyncApp : public CWinApp
{
public:
	CEcuFileSyncApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CEcuFileSyncApp theApp;