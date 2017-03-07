// TEST.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "EcuFileSync.h"
#include "TEST.h"
#include "afxdialogex.h"


// TEST 대화 상자입니다.

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


// TEST 메시지 처리기입니다.
