#pragma once


// TEST ��ȭ �����Դϴ�.

class TEST : public CDialogEx
{
	DECLARE_DYNAMIC(TEST)

public:
	TEST(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~TEST();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};
