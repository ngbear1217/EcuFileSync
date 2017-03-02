#include "stdafx.h"
#include "ClientObject.h"
#include "resource.h"


ClientObject::ClientObject()
{
	pMainWnd = AfxGetApp()->GetMainWnd();
	m_nConnectType = CONNECT_TYPE::NEW_CONNECT;
	m_nPacketCurrentSize = 0;
	m_nPacketAllSize = 0;
	m_FileType = FILE_TYPE::IDLE;
	m_bUpdateResult = false;
	m_bFileMonitoringYn = true;
	m_bChkSendPackTime = false;

	char strCurPath[256] = { 0, };
	CString strINIPath;
	GetCurrentDirectory(sizeof(strCurPath), strCurPath);
	strINIPath.Format("%s\\%s", strCurPath, INI_PATH);
	
	char cbuf[256] = { 0, };
	GetPrivateProfileString("CONFIGURATION_INFO", "ECUFILE_PATH", "", cbuf, sizeof(cbuf), strINIPath);
	m_strDirPath.Format("%s", cbuf);
	GetPrivateProfileString("CONFIGURATION_INFO", "ECU_MASTERDB_PATH", "", cbuf, sizeof(cbuf), strINIPath);
	m_strMasterDBDirPath.Format("%s", cbuf);

	pCDBMng = new CDBManager();
	DWORD dwThreadId = 0;
	m_dwMonitoringTime = GetTickCount();
	m_dwSendPackTime = 0;
	m_strMessage = "";
#ifdef USING_THREAD
	hThreadMonitor1 = CreateThread(NULL, 0, MonitoringThread, (LPVOID)this, 0, &dwThreadId);
	hThreadMonitor2 = CreateThread(NULL, 0, PacketTimeThread, (LPVOID)this, 0, &dwThreadId);
#endif
}
ClientObject::~ClientObject()
{
	m_bFileMonitoringYn = false;
	m_FileType = FILE_TYPE::DEAD;
#ifdef USING_THREAD
	WaitForSingleObject(hThreadMonitor1, 1000);
	WaitForSingleObject(hThreadMonitor2, 1000);
	CloseHandle(hThreadMonitor1);
	CloseHandle(hThreadMonitor2);
#endif
	delete pCDBMng; 
	CloseSocketCtx(m_pPerSocketCtx);
	RemoveClient();
}

void ClientObject::CloseSocketCtx(PPerSocketContext pPerSocketCtx)
{
	if (pPerSocketCtx != NULL)
	{
	//	Lock();
		shutdown(pPerSocketCtx->recvSocket, SD_BOTH);
		shutdown(pPerSocketCtx->sendSocket, SD_BOTH);
		closesocket(pPerSocketCtx->recvSocket);
		closesocket(pPerSocketCtx->sendSocket);
		//pPerSocketCtx->socket=INVALID_SOCKET;
		if (pPerSocketCtx->recvContext != NULL)
		{
			delete pPerSocketCtx->recvContext;
			pPerSocketCtx->recvContext = NULL;
		}
		if (pPerSocketCtx->sendContext != NULL)
		{
			delete pPerSocketCtx->sendContext;
			pPerSocketCtx->sendContext = NULL;
		}
		delete pPerSocketCtx;
		pPerSocketCtx = NULL;
		U_SubLog("COMMON", m_cstrIP, "[%-40s]del RemoveClient SocketObj", __FUNCTION__);
		//TRACE("\nCloseSocketCtx\n");
	//	unLock();
	}
}
void ClientObject::RemoveClient(){
	
	try{
		if (pMainWnd != NULL)
		{
			CProgressCtrl* pProgressBar = (CProgressCtrl*)(pMainWnd->GetDlgItem(unProgressBar));
			pProgressBar->SetPos(0);
			((CStatic*)pMainWnd->GetDlgItem(unStaticLed))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LED_OFF)));
			pMainWnd->SetDlgItemTextA(unStaticIP, "");
			pMainWnd->SetDlgItemTextA(unStaticBoot, "");
			pMainWnd->SetDlgItemTextA(unStaticFw, "");
			pMainWnd->SetDlgItemTextA(unView, "");
			//pMainWnd->SetDlgItemTextA(unStaticIP, "");
			pMainWnd->CheckDlgButton(unChkBox, false);
			CString strTmp, strTmp2;
			pMainWnd->GetDlgItemTextA(IDC_EDIT_MAINVIEW, strTmp);
			strTmp2.Format("%s \r\n %s Disconnect ", strTmp, m_cstrIP);
			pMainWnd->SetDlgItemTextA(IDC_EDIT_MAINVIEW, strTmp2);
						

		}
	}
	catch (DWORD err){

	}
	U_Log("COMMON", "Client(IP:%s) Disconnected", m_cstrIP);


}

//////////////////////////////////// Common start /////////////////////////////////////
bool ClientObject::SetSendPacket(unsigned char cCmdID, unsigned short nLength)
{
	if (!(m_FileType >= FILE_TYPE::IDLE && m_FileType < FILE_TYPE::DEAD))
		return false;
	char cChkSum = 0;
	m_nWaitCmd = cCmdID;
	PBASIC_PACKET pPacket = (PBASIC_PACKET)(m_pPerSocketCtx->sendContext->Buffer);
	pPacket->cSource = TERMINALID::SERVER;
	pPacket->cDestination = TERMINALID::DEVICE;
	pPacket->cCmdID = cCmdID;
	ShortToByte(nLength, pPacket->nLength);

	cChkSum = CalculateChkSum((char*)pPacket, nLength + SIZE_HEADER);

	if (MAX_BUFFER == nLength)
	{
		pPacket->cCheckSum = cChkSum;
	}
	else
	{
		pPacket->pData[nLength] = cChkSum;
	}
	m_pPerSocketCtx->sendContext->wsaBuf.len = nLength + SIZE_HEADER_REAR;
	m_pPerSocketCtx->sendContext->wsaBuf.buf = m_pPerSocketCtx->sendContext->Buffer;

	m_bChkSendPackTime = true;
	m_dwSendPackTime = GetTickCount();
	m_pCallbackForSend(m_pPerSocketCtx);
	return true;

}
char ClientObject::CalculateChkSum(char* pBuf, int nLength)
{
	char cChkSum = 0;
	for (int i = 0; i<nLength; i++)
	{
		cChkSum ^= pBuf[i];
	}
	return cChkSum;
}
void ClientObject::ClearSendPacket()
{
	if ((DWORD)(m_pPerSocketCtx->sendContext) == 0xfeeefeee)
		return;
	memset(m_pPerSocketCtx->sendContext->Buffer, 0, MAX_BUFFER);

}
void ClientObject::ClearReceivePacket()
{
	memset(cPacketBuf, 0, MAX_BUFFER);
}

/********* util function ***************/
CString ClientObject::GetCRC(CFile* _cFile){
	/*CFile file;
	CFileException fileException;
	if (!file.Open(strFilePath, CFile::modeNoTruncate | CFile::modeReadWrite, &fileException))
	{
	return "";
	}*/

	CString strResult = "";
	char pBuf[1] = { 0x00 };

	UINT cu4_CRC32_table[256] =
	{
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	if (INVALID_HANDLE_VALUE != _cFile)
	{
		UINT tu4_Result;

		tu4_Result = (UINT)0xFFFFFFFF;

		for (int i = 0; i < (int)_cFile->GetLength(); i++)
		{
			_cFile->Read(pBuf, 1);
			tu4_Result = cu4_CRC32_table[(tu4_Result ^ (UINT)(pBuf[0])) & (UINT)0x000000FF] ^ (tu4_Result >> 8);
		}

		tu4_Result = tu4_Result ^ (UINT)0xFFFFFFFF;
		strResult.Format("%08x", tu4_Result);
	}
	return strResult;
}
CString ClientObject::GetCRC(CString strFilePath)
{
	CFile file;
	CFileException fileException;
	if (!file.Open(strFilePath, CFile::modeNoTruncate | CFile::modeReadWrite, &fileException))
	{
		return "";
	}

	CString strResult = "";
	char pBuf[1] = { 0x00 };

	UINT cu4_CRC32_table[256] =
	{
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	if (INVALID_HANDLE_VALUE != file)
	{
		UINT tu4_Result;

		tu4_Result = (UINT)0xFFFFFFFF;

		for (int i = 0; i < (int)file.GetLength(); i++)
		{
			file.Read(pBuf, 1);
			tu4_Result = cu4_CRC32_table[(tu4_Result ^ (UINT)(pBuf[0])) & (UINT)0x000000FF] ^ (tu4_Result >> 8);
		}

		tu4_Result = tu4_Result ^ (UINT)0xFFFFFFFF;
		strResult.Format("%08x", tu4_Result);
	}
	return strResult;
}
void ClientObject::intToByte(int value, byte data[], int idx)
{
	data[idx] = (byte)(value >> 24);
	data[++idx] = (byte)(value >> 16);
	data[++idx] = (byte)(value >> 8);
	data[++idx] = (byte)value;



}
void ClientObject::ShortToByte(unsigned short usNum, BYTE* bytes)
{
	bytes[1] = usNum & 0xff;
	bytes[0] = usNum >> 8;
}
int ClientObject::ByteToShort(BYTE* bytes)
{
	unsigned short usNum = 0;
	usNum = (usNum << 8) + bytes[0];
	usNum = (usNum << 8) + bytes[1];

	return usNum;
}
int ClientObject::ByteToInt(BYTE bytes[4])
{
	int s1 = bytes[0] & 0xFF;
	int s2 = bytes[1] & 0xFF;
	int s3 = bytes[2] & 0xFF;
	int s4 = bytes[3] & 0xFF;

	return ((s1 << 24) + (s2 << 16) + (s3 << 8) + (s4 << 0));
}
void ClientObject::HexString2Int(CString strHex, unsigned char* cBuf, int nLen)

{
	int nTemp = 0;
	//unsigned char cbuf[4] = { 0, };
	memset(cBuf, 0x00, nLen);
	CString strTmp;
	for (int i = 0; i < nLen; i++)
	{
		strTmp = strHex.Mid(i * 2, 2);
		sscanf(strTmp, "%x", &nTemp);
		cBuf[i] = nTemp;
	}
	//return nTemp;

}
//////////////////////////////////// send packet start /////////////////////////////////////

/********* SendPacketForFileInfo  업데이트파일 정보 전송 ***************/
void ClientObject::SendPacketForFileInfo(CString strServerFilePath)  // read, write  둘다 여기로 옮.
{
	ClearSendPacket();
	PBASIC_PACKET pPacket = (PBASIC_PACKET)(m_pPerSocketCtx->sendContext->Buffer);
	int nLen = 0;
	m_strServerFilePath = strServerFilePath;


	PSTRT_FILE_INFO _strt_file_info = new STRT_FILE_INFO();
	memset(_strt_file_info, 0, sizeof(STRT_FILE_INFO));

	CString strChkSum;
	CFileException fileException;
	CFile f_ServerFile;
	int nFileSize = 0;

	//file size
	if (m_FileType == FILE_TYPE::IDLE){
		if (!f_ServerFile.Open(m_strServerFilePath, CFile::modeRead | CFile::shareDenyNone, &fileException)){
			U_SubLog("COMMON", m_cstrIP, "[%-40s][ERROR] Failed to open file for Write, [%s]", __FUNCTION__, m_strServerFilePath);
			m_strMessage.Format("Can't open file %s, error = %u\n", m_strServerFilePath, fileException.m_cause);
			//	m_FileType = FILE_TYPE::IDLE;
			if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);
		}
		else{
			m_FileType = FILE_TYPE::UPDATE_STEP1;
			m_strServerFileName = f_ServerFile.GetFileName();
			int nCopyLen = m_strServerFileName.GetLength();
			memcpy(_strt_file_info->FileName, m_strServerFileName, nCopyLen);//file path 입력

			nFileSize = (int)f_ServerFile.GetLength();
			intToByte(nFileSize, _strt_file_info->FileSize, 0);//file size 입력

			//strChkSum = GetCRC(m_strServerFilePath);
			strChkSum = GetCRC(&f_ServerFile);
			unsigned char cBuf_ChkSum[4];
			HexString2Int(strChkSum, cBuf_ChkSum, 4);

			
			memcpy(_strt_file_info->FileCheckSum, cBuf_ChkSum, 4);//checksum 입력
			
			memcpy(pPacket->pData, _strt_file_info, sizeof(STRT_FILE_INFO));

			f_ServerFile.Close();
			
			SetSendPacket(CMDID::FID_PV_SENDFILEINFO, sizeof(STRT_FILE_INFO));
			m_strMessage.Format("Sendind : %s", m_strServerFileName);
			U_SubLog("COMMON", m_cstrIP, "[%-40s]SendPacketForFileInfo ", __FUNCTION__);
			if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);
		}
	}
	else{
		U_SubLog("COMMON", m_cstrIP, "[%-40s][ERROR] File Object is bussy  [%s]", __FUNCTION__, m_strServerFilePath);
		m_strMessage.Format("File Object is bussy");
		if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);
	}
}
/********* SendPacketForFileStream 파일 DATA 전송 **************/
void ClientObject::SendPacketForFileStream(){
	ClearSendPacket();
	//m_nStartTime = GetTickCount();
	PBASIC_PACKET pPacket = (PBASIC_PACKET)(m_pPerSocketCtx->sendContext->Buffer);
	int nLen = 0;
	//m_nFileSendCnt = 0;

	CFileException fileException;
	CFile f_ServerFile;
	int nTmpPercent = 0;
	int nFileLength = 0;
	int nFileSendCnt = 0;
	int nLastFileDataSize = 0;
	int nFileSendPosition = 0;

	m_bUpdateResult = false;
	m_FileType = FILE_TYPE::UPDATE_STEP2;

	if (!f_ServerFile.Open(m_strServerFilePath, CFile::modeRead | CFile::shareDenyNone, &fileException))
	{
		m_FileType = FILE_TYPE::IDLE;
		U_SubLog("COMMON", m_cstrIP, "[%-40s][ERROR] Failed to open file for Write, [%s]", __FUNCTION__, m_strServerFilePath);
		m_strMessage.Format("Can't open file %s, error = %u\n", m_strServerFilePath, fileException.m_cause);
		if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);
	}
	else
	{
		nFileLength = f_ServerFile.GetLength();
		nFileSendCnt = nFileLength / FILE_WRITING_SIZE;
		nLastFileDataSize = nFileLength % FILE_WRITING_SIZE;//mod
		nFileSendPosition = 0;
		CWnd* pWnd = AfxGetApp()->GetMainWnd();
		CProgressCtrl* pProgressBar = (CProgressCtrl*)pWnd->GetDlgItem(unProgressBar);

		BYTE buffer[FILE_WRITING_SIZE];

		DWORD dwRead;

		if (nLastFileDataSize>0)
		{
			nFileSendCnt++;
		}

		for (int i = 0; i<nFileSendCnt; i++)
		{
			memset(buffer, 0x00, FILE_WRITING_SIZE);
			dwRead = f_ServerFile.Read(buffer, FILE_WRITING_SIZE);
			ClearSendPacket();
			BYTE* pSendData = (BYTE*)GetSendDataBuffer();
			int cnt = 0;
			pSendData[cnt++] = (unsigned char)((i & 0xFF000000) >> 24);
			pSendData[cnt++] = (unsigned char)((i & 0x00FF0000) >> 16);
			pSendData[cnt++] = (unsigned char)((i & 0x0000FF00) >> 8);
			pSendData[cnt++] = (unsigned char)((i & 0x000000FF));
			pSendData[cnt++] = (unsigned char)((dwRead & 0x0000FF00) >> 8);
			pSendData[cnt++] = (unsigned char)((dwRead & 0x000000FF));
			memcpy(&pSendData[cnt], buffer, dwRead);

			//U_Log("TEST", "[file6]Send WriteFile");
			SetSendPacket(CMDID::FID_PV_SENDFILESTREAM, dwRead + 6);

			
			int percent = int((nFileSendPosition * 100)) / nFileLength;

			nFileSendPosition = nFileSendPosition + dwRead;
			
			pProgressBar->SetPos(percent);
#ifdef VER3_1_3_0
			if (percent > nTmpPercent){
				m_strMessage.Format("File Download : %d%%", percent);
				if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_UI_MESSAGE, NULL, (LPARAM)this);
				nTmpPercent = nTmpPercent + 10;
			}
#endif
			U_SubLog("COMMON", m_cstrIP, "[%-40s][SEND] CMDID::FID_PV_WRITEFILE. FileSendCnt:%d(%d) FileSendByte:%d (%d%)", __FUNCTION__, nFileSendCnt, i, nFileSendPosition, percent);
		}
		pProgressBar->SetPos(100);
		f_ServerFile.Close();
		U_SubLog("COMMON", m_cstrIP, "[%-40s][FILE] WRITE FILE Close.  filesize : %d, filesendCnt:%d ", __FUNCTION__, nFileLength, nFileSendCnt);
		Sleep(10);
		SendPacketForFileEndChk();

	}
}
/********* SendPacketForFileEndChk  파일 전송 완료 체크 전송 ***************/
void ClientObject::SendPacketForFileEndChk()
{
	//U_Log("TEST", "[file8]Send CloseFile");
	m_FileType = FILE_TYPE::UPDATE_STEP3;
	ClearSendPacket();
	U_SubLog("COMMON", m_cstrIP, "[%-40s][SEND] FID_PV_CLOSEFILE .", __FUNCTION__);

	SetSendPacket(CMDID::FID_PV_SENDFILECHECK, 1);


}

/////////////////////////////////////////// RECV packet start ////////////////////////////////////////////////////////////

/********* ProcessPacket  받은 패킷 처리 ***************/
int ClientObject::ProcessPacket()
{
	PBASIC_PACKET pPacket = (PBASIC_PACKET)cPacketBuf;

	//m_dwAliveBitTime = GetTickCount();
	int nCheckResult = CheckPacketValidation();
	////TRACE("\n ERR CHK : nCheckResult %d ",nCheckResult);
	if (nCheckResult>-1)
	{
		int nRet = 0;
		switch (pPacket->cCmdID)
		{
		
		case CMDID::FID_PV_SENDFILEINFO:
			if (m_FileType == FILE_TYPE::UPDATE_STEP1)
				ReceivedSendFileInfoResponse();//Open -> read or write
			break;

		case CMDID::FID_PV_SENDFILECHECK:
			if (m_FileType == FILE_TYPE::UPDATE_STEP3)
				ReceivedSendFileCheckResponse();
			break;
		//case CMDID::FID_PV_READFILE:
		//	ReceivedReadFileResponse();
		//	break;
		//case CMDID::FID_PV_WRITEFILE:
		//	//U_Log("TEST", "[file7]Recv WriteFile");
		//	//	ReceivedWriteFileResponse();
		//	break;
		//case CMDID::FID_PV_CLOSEFILE:
		//	ReceviedCloseFileResponse();
		//	break;
		//case CMDID::FID_PV_CHECKSUMFILE:
		//	//SendPacketForFileEndChk();
		//	ReceviedCheckSumFileResponse();
		//	break;
		//case CMDID::FID_PV_DELETEFILE:
		//	if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_DELETEFILE, NULL, (LPARAM)this);
		//	break;
		//

		}
	}
	else
	{
		if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);
	}
	return 0;
}
/********* CheckPacketValidation  받은 패킷 유효성 체크 ***************/
int ClientObject::CheckPacketValidation()
{
	//char* st = m_cstrIP;

	//PBASIC_PACKET pPacket = (PBASIC_PACKET) (cPacketBuf);
	PBASIC_PACKET pPacket = (PBASIC_PACKET)(cPacketBuf);
	if (pPacket->cDestination != TERMINALID::SERVER || pPacket->cSource != TERMINALID::DEVICE)
	{
		m_strMessage.Format("[ERROR] Unmatched terminal");
		U_SubLog("COMMON", m_cstrIP, "[%-40s]%s", __FUNCTION__, m_strMessage);
		return -1; // Unmatched terminal
	}

	unsigned short nLen = ByteToShort(pPacket->nLength);
	if (nLen > MAX_BUFFER)
	{
		m_strMessage.Format("[ERROR] Length is wrong. Len(%d)", nLen);
		U_SubLog("COMMON", m_cstrIP, "[%-40s]%s", __FUNCTION__, m_strMessage);
		return -2; // Length is wrong
	}

	unsigned char cCompareChkSum = 0;
	if (nLen == MAX_BUFFER)
	{
		cCompareChkSum = pPacket->cCheckSum;
	}
	else
	{
		cCompareChkSum = pPacket->pData[nLen];
	}

	unsigned char cChkSum = CalculateChkSum(cPacketBuf, nLen + SIZE_HEADER);
	if (cChkSum != cCompareChkSum)
	{
		m_strMessage.Format("[ERROR] Unmatched checksum. Received ChkSum(%02X) Calculated ChkSum(%02X)", cCompareChkSum, cChkSum);
		U_SubLog("COMMON", m_cstrIP, "[%-40s]%s", __FUNCTION__, m_strMessage);

		return -3;
	}
	if (pPacket->cCmdID == CMDID::FID_ALL_NAK)
	{
		m_strMessage.Format("[ERROR] NAK. Received 7f CMD : %02X, ERROR_CD : %02X, IP : %s", pPacket->pData[nLen - 2], pPacket->pData[nLen - 1], m_cstrIP);
		U_SubLog("COMMON", m_cstrIP, "[%-40s]%s", __FUNCTION__, m_strMessage);
	}
	return 0;

}
/********* ReceivedSendFileInfoResponse  업데이트파일 정보전송에 대한 ACK 처리 ***************/
void ClientObject::ReceivedSendFileInfoResponse(void)
{
	m_strMessage.Format("Sending File :%s", m_strServerFileName);
	U_SubLog("COMMON", m_cstrIP, "[%-40s]FileStreamStart , FileName : %s ", __FUNCTION__, m_strServerFileName);
	
	if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);

	SendPacketForFileStream();
}
/********* ReceivedSendFileInfoResponse  파일전송완료 체크전송에 대한 ACK 처리 ***************/
void ClientObject::ReceivedSendFileCheckResponse(){
	PBASIC_PACKET pPacket = (PBASIC_PACKET)(cPacketBuf);
	int nOkChk = 0;
	nOkChk = (int)pPacket->pData[0];
		
	if (nOkChk == 1)//ok
	{
		m_bUpdateResult = true;
		}
	else{ //ng
		m_bUpdateResult = false;
	}
	m_FileType = FILE_TYPE::IDLE;
	SetFileUpResultInfo();
	
}

/////////////////////////////////////////// VIEW Fn start ////////////////////////////////////////////////////////////

/********* view Function ***************/
void ClientObject::SetStatic(int VciNo, bool bChkNewBoard){
	mViewNo = VciNo;

	//int nTmp;
	//nTmp = atoi(m_strIP.Right(3));
	switch (VciNo){
	case VIEW_NO::BOARD1:
		unStaticLed = IDC_STC_VCI1_LED;
		unStaticIP = IDC_STC_VCI1_IP;
		unProgressBar = IDC_PROGRESS1;
		unChkBox = IDC_CHK_VCI1;
		unView = IDC_EDT_VIEW1;
		break;
	case VIEW_NO::BOARD2:
		unStaticLed = IDC_STC_VCI2_LED;
		unStaticIP = IDC_STC_VCI2_IP;
		unProgressBar = IDC_PROGRESS2;
		unChkBox = IDC_CHK_VCI2;
		unView = IDC_EDT_VIEW2;
		break;
	case VIEW_NO::BOARD3:
		unStaticLed = IDC_STC_VCI3_LED;
		unStaticIP = IDC_STC_VCI3_IP;
		unProgressBar = IDC_PROGRESS3;
		unChkBox = IDC_CHK_VCI3;
		unView = IDC_EDT_VIEW3;
		break;
	case VIEW_NO::BOARD4:
		unStaticLed = IDC_STC_VCI4_LED;
		unStaticIP = IDC_STC_VCI4_IP;
		unProgressBar = IDC_PROGRESS4;
		unChkBox = IDC_CHK_VCI4;
		unView = IDC_EDT_VIEW4;
		break;
	case VIEW_NO::BOARD5:
		unStaticLed = IDC_STC_VCI5_LED;
		unStaticIP = IDC_STC_VCI5_IP;
		unProgressBar = IDC_PROGRESS5;
		unChkBox = IDC_CHK_VCI5;
		unView = IDC_EDT_VIEW5;
		break;
	case VIEW_NO::BOARD6:
		unStaticLed = IDC_STC_VCI6_LED;
		unStaticIP = IDC_STC_VCI6_IP;
		unProgressBar = IDC_PROGRESS6;
		unChkBox = IDC_CHK_VCI6;
		unView = IDC_EDT_VIEW6;
		break;
	case VIEW_NO::BOARD7:
		unStaticLed = IDC_STC_VCI7_LED;
		unStaticIP = IDC_STC_VCI7_IP;
		unProgressBar = IDC_PROGRESS7;
		unChkBox = IDC_CHK_VCI7;
		unView = IDC_EDT_VIEW7;
		break;
	case VIEW_NO::BOARD8:
		unStaticLed = IDC_STC_VCI8_LED;
		unStaticIP = IDC_STC_VCI8_IP;
		unProgressBar = IDC_PROGRESS8;
		unChkBox = IDC_CHK_VCI8;
		unView = IDC_EDT_VIEW8;
		break;
	case VIEW_NO::BOARD9:
		unStaticLed = IDC_STC_VCI9_LED;
		unStaticIP = IDC_STC_VCI9_IP;
		unProgressBar = IDC_PROGRESS9;
		unChkBox = IDC_CHK_VCI9;
		unView = IDC_EDT_VIEW9;
		break;
	case VIEW_NO::BOARD10:
		unStaticLed = IDC_STC_VCI10_LED;
		unStaticIP = IDC_STC_VCI10_IP;
		unProgressBar = IDC_PROGRESS10;
		unChkBox = IDC_CHK_VCI10;
		unView = IDC_EDT_VIEW10;
		break;
	case VIEW_NO::BOARD11:
		unStaticLed = IDC_STC_VCI11_LED;
		unStaticIP = IDC_STC_VCI11_IP;
		unProgressBar = IDC_PROGRESS11;
		unChkBox = IDC_CHK_VCI11;
		unView = IDC_EDT_VIEW11;
		break;
	}
}



void ClientObject::GetFileInfo()
{
	//CString strDeviceIp = .Right(3);
	
	CString strServerFilePath, strFileName;

	if (pCDBMng->GetFileInfo(m_strIP)){ //update file 정보가 없으면 false
#ifdef SEND_FILEINFO_TO_DEVICE	
		if (pCDBMng->mCurFileInfo.FileSeq > 0){

			strFileName.Format("%s", pCDBMng->mCurFileInfo.FileName);
			int a = strFileName.Find(".xml");
			int b = strFileName.Find(".XML");
			if (strFileName.Find(".xml") > 0 || strFileName.Find(".XML") > 0){//masterDB에 대한 경로 변경
				strServerFilePath.Format("%s\\%s", m_strMasterDBDirPath, pCDBMng->mCurFileInfo.FileName);
			}
			else{
				strServerFilePath.Format("%s\\%s", m_strDirPath, pCDBMng->mCurFileInfo.FileName);
			}
			SendPacketForFileInfo(strServerFilePath);
		}
#endif
	}
#ifdef SEND_FILEINFO_TO_DEVICE	
	m_strMessage = pCDBMng->m_strMessage;
	if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);
#endif
	
}


void ClientObject::SetFileUpResultInfo()
{
	bool bTmp = false;
	if (m_bUpdateResult){
		bTmp = pCDBMng->SetFileUpResultInfo(m_strIP, pCDBMng->mCurFileInfo.FileSeq, 'Y');
		m_strMessage.Format("FILE UPLOAD OK");
		U_SubLog("COMMON", m_cstrIP, "[%-40s]FileUpload OK , FileName : %s ", __FUNCTION__, m_strServerFileName);
	}
	else{
		bTmp = pCDBMng->SetFileUpResultInfo(m_strIP, pCDBMng->mCurFileInfo.FileSeq, 'N');
		m_strMessage.Format("FILE UPLOAD NG");
		U_SubLog("COMMON", m_cstrIP, "[%-40s]FileUpload NG , FileName : %s ", __FUNCTION__, m_strServerFileName);
	}
	
	if (!bTmp){
		m_bUpdateResult = false;
		m_strMessage = pCDBMng->m_strMessage;
	}
	
	if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_SUBMSG, NULL, (LPARAM)this);

	GetFileInfo();
	//if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_FILEUP_CHK, NULL, (LPARAM)this);
}



DWORD WINAPI MonitoringThread(PVOID param)// 전달 파일 유무 모니터링
{
	ClientObject* pClientObj = (ClientObject*)param;

	DWORD dwTime = 0;
	DWORD dwCurrentTime = 0;
	pClientObj->m_dwMonitoringTime = GetTickCount();
	while (true){
		Sleep(1000);
		if (!pClientObj->m_bFileMonitoringYn)
			break;

		if (pClientObj->m_FileType == FILE_TYPE::IDLE){
			dwCurrentTime = GetTickCount();
			if ((dwCurrentTime - pClientObj->m_dwMonitoringTime) > MONITORING_TIME * 1000){
				U_SubLog("COMMON", pClientObj->m_cstrIP, "[%-40s]UPDATE START BY THREAD ", __FUNCTION__);
				pClientObj->m_dwMonitoringTime = GetTickCount();
#ifdef CHKMEM_GetFileInfo
				pClientObj->GetFileInfo();
#endif	
			}
		}
	}
	return 0;
}
DWORD WINAPI PacketTimeThread(PVOID param)// 전달 파일 유무 모니터링
{
	ClientObject* pClientObj = (ClientObject*)param;
	DWORD dwTime = 0;
	while (true){
		Sleep(1000);
		if (!pClientObj->m_bFileMonitoringYn)
			break;
		if (pClientObj->m_bChkSendPackTime){
			//Sleep(100);
			dwTime = GetTickCount();
			if (pClientObj->m_dwSendPackTime == 0) continue;
			if (dwTime - (pClientObj->m_dwSendPackTime) > PACKETRETURN_TIME * 1000){

				U_SubLog("COMMON", pClientObj->m_cstrIP, "[%-40s] ******* PACKETRETURN Time out !!!!", __FUNCTION__);
				pClientObj->m_FileType = FILE_TYPE::IDLE;
				pClientObj->m_bChkSendPackTime = false;
			}
		}
	}
		return 0;

	}

