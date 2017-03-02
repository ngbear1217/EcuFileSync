#pragma once
#include "ProtocolDef.h"
#include "DBManager.h"

typedef bool(*pCallbackParam1)(PPerSocketContext pPerSocketCtx);
class ClientObject
{
public:
	ClientObject();
	~ClientObject();


	void SetCallBackForSend(pCallbackParam1 pCallback) { m_pCallbackForSend = pCallback; }//callback SendPack 호출
	void SetPerSocketCtx(PPerSocketContext pPerSocketCtx){ m_pPerSocketCtx = pPerSocketCtx; };//
	
public://common
	CWnd* pMainWnd;
	CDBManager* pCDBMng;

	
	UINT m_nWaitCmd;
	CString m_strMessage;
	CString m_strDirPath;
	CString m_strMasterDBDirPath;

	int ByteToShort(BYTE* bytes);
	void ShortToByte(unsigned short usNum, BYTE* bytes);
	void intToByte(int value, byte data[], int idx);
	int ByteToInt(BYTE bytes[4]);
	CString GetClientIP(){ return m_strIP; };
	CString GetCRC(CString strFilePath);
	CString GetCRC(CFile* _cFile);
	void HexString2Int(CString strHex, unsigned char* cBuf, int nLen);

public ://view
	UINT mViewNo;
	UINT unStaticLed;
	UINT unStaticIP;
	UINT unStaticSerial;
	UINT unStaticBoot;
	UINT unStaticFw;
	UINT unProgressBar;
	UINT unChkBox;
	UINT unView;

	void ClientObject::SetStatic(int VciNo, bool bChkNewBoard);
	//UINT GetLedId(){ return unStaticLed; };
	//UINT GetFWId(){ return unStaticFw; };
	//UINT GetBootId(){ return unStaticBoot; };
	//UINT GetIPId(){ return unStaticIP; };
	//UINT GetSerial(){ return unStaticSerial; };
	//UINT GetBarId(){ return unProgressBar; };
	//UINT GetChkBoxId(){ return unChkBox; };
	//UINT GetViewId(){ return unView; };

public://connect
	//CString m_strMessage;
	char m_cstrIP[16];
	CString m_strIP;
	int m_nConnectType;
	bool m_bFileMonitoringYn;
	bool m_bChkSendPackTime;
	HANDLE hThreadMonitor1;
	HANDLE hThreadMonitor2;
	DWORD m_dwMonitoringTime;
	DWORD m_dwSendPackTime;
	void RemoveClient();
	void CloseSocketCtx(PPerSocketContext pPerSocketCtx);

public://packet

	char cPacketBuf[MAX_BUFFER];
	DWORD m_nPacketCurrentSize;
	DWORD m_nPacketAllSize;
	DWORD m_nStartTime;
	DWORD m_nTotalTime;
	bool m_bUpdateResult;

	CString m_strServerFilePath;
	CString m_strServerFileName;
	//int m_nFileSize;
	//int m_nFileSendCnt;
	int m_FileType;
	//CFile m_File;
	//CFileException m_fileException;
	//DWORD m_nCurrentFileSize;
	//DWORD m_nFileLength;
	bool SetSendPacket(unsigned char cCmdID, unsigned short nLength);
	char CalculateChkSum(char* pBuf, int nLength);
	int ProcessPacket();// Revc Packet 처리 	
	int CheckPacketValidation();// Packet validation check
	void ClearSendPacket();//SendPacket reset
	void ClearReceivePacket(); //clear ReceivePacket
	/**********SEND**************/
	void SendPacketForFileInfo(CString strServerFilePath);
	void SendPacketForFileStream();
	void SendPacketForFileEndChk();

	/***********RECV*******************/
	void ReceivedSendFileInfoResponse(void);
	void ReceivedSendFileCheckResponse();
	
private:
	PPerSocketContext m_pPerSocketCtx;
	pCallbackParam1 m_pCallbackForSend;

	unsigned char* GetSendDataBuffer() { return ((PBASIC_PACKET)(m_pPerSocketCtx->sendContext->Buffer))->pData; }

public:
	void GetFileInfo();
	void SetFileUpResultInfo();
};


DWORD WINAPI MonitoringThread(PVOID param);
DWORD WINAPI PacketTimeThread(PVOID param);