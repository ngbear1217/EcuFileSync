// TEST.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "EcuFileSync.h"
#include "TEST.h"
#include "afxdialogex.h"


// TEST ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(TEST, CDialogEx)

TEST::TEST(CWnd* pParent /*=NULL*/)
	: CDialogEx(TEST::IDD, pParent)
{

}

TEST::~TEST()
{
}

void TEST::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TEST, CDialogEx)
END_MESSAGE_MAP()


// TEST �޽��� ó�����Դϴ�.
