#pragma once
#include "ProtocolDef.h"
//
//#ifdef USING_ADO15
//#ifndef WIN64
//#import "C:\Program Files\Common Files\System\ado\msado15.dll"  rename("EOF", "EndOfFile") 
//#else
//#import "C:\Program Files (x86)\Common Files\System\ado\msado15.dll"  rename("EOF", "EndOfFile") 

//#endif

typedef ADODB::_RecordsetPtr  RecPtr;
typedef ADODB::_ConnectionPtr CnnPtr;

//#endif
class CDBManager
{
public:
	CDBManager();
	~CDBManager();


protected:
	BOOL OpenDB(void);
	void CloseDB(void);

private:
	void GetConnString(CString& strConn);

protected:
	static CString m_strDBIP;
	static CString m_strDBName;

	const CString m_strID;
	const CString m_strPW;
	const CString m_strPort;

	CnnPtr m_pConn;
	RecPtr m_Rec;
	//_CommandPtr m_pCmd;

public:

	CString m_strMessage;
	static void SetDBInfo(CString& strIP, CString& strDBName);

	//bool GetFileInfoFromDB();
	bool GetFileInfo(CString strDevice_IP);
	bool SetFileUpResultInfo(CString strDevice_IP, int nFileSeq, char cResult);
	STRT_UPDATE_FILE_INFO mCurFileInfo;
};

