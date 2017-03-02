#include "StdAfx.h"
//#include "iocpTest3.h"
#include "NetworkController.h"
#include <assert.h>
#include  <Mstcpip.h>




// 디버그 출력
bool bMainClose = false;


NetworkController::NetworkController(void)
{
	m_listenSocket = NULL;
	hMutex = CreateMutex(NULL, FALSE, NULL);
	pMainWnd = AfxGetApp()->GetMainWnd();
	nTest = 0;
}

NetworkController::~NetworkController(void)
{
	bMainClose = true;
	ServerClose();

	Sleep(1000);
	WSACleanup();
	//DeleteCriticalSection(&m_cs);
	m_Objects.clear();
	U_Log("COMMON", "Destroy NetworkController");
	CloseHandle(hMutex);
}

// 초기화 처리
bool NetworkController::Init(const int nPort)
{
	WSADATA wsd;
	SOCKADDR_IN serverSockAddr;

	// 소켓 초기화 처리 
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		U_Log("COMMON", "[ERROR]WSAStartup Failed: %d\n", WSAGetLastError());
		return false;
	}

	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		U_Log("COMMON", "[ERROR]Listen Socket Creation Failed: %d\n", WSAGetLastError());
		return false;
	}

	serverSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_port = htons((short)nPort);
	if (bind(m_listenSocket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr)))
	{
		U_Log("COMMON", "[ERROR]Bind Failed: %d\n", WSAGetLastError());
		closesocket(m_listenSocket);
		m_listenSocket = NULL;
		return false;
	}

	//	struct tcp_keepalive keepAlive;  
	//
	//	keepAlive.onoff = 1;   
	//	keepAlive.keepalivetime = 3000; // 3초   
	//	keepAlive.keepaliveinterval = 500; // 0.5 초 
	//	  
	////	DWORD dwBytesReturned; 
	//	// KEEP_ALIVE 옵션 설정  
	//	
	//	WSAIoctl( m_listenSocket   
	//		, SIO_KEEPALIVE_VALS  
	//		, &keepAlive  
	//		, sizeof(tcp_keepalive)  
	//		, 0, 0   
	//		, &dwBytesReturned, NULL, NULL );  


	if (SOCKET_ERROR == listen(m_listenSocket, SOMAXCONN))
	{
		U_Log("COMMON", "[ERROR]Change to Listen Mode Error: %d\n", WSAGetLastError());
		closesocket(m_listenSocket);
		m_listenSocket = NULL;
		return false;
	}

	U_Log("COMMON", "Socket Initiation Success\n");

	// IOCP 초기화 처리 
	int ErrCode;
	if (!m_IocpHandler.Create(0, &ErrCode))
	{
		U_Log("COMMON", "[ERROR]Create IO Completion Port Error: %d\n", ErrCode);
		return false;
	}
	//IOCP workThread 생성
	if (!m_IocpHandler.CreateThreadPool(this))
	{
		U_Log("COMMON", "[ERROR]Create Thread Pool Failed\n");
		return false;
	}

	U_Log("COMMON", "IOCP Initiation Success\n");
	return true;
}

// Accept 작업 처리

void NetworkController::AcceptProcess(void)
{
	int ErrCode = 0;
	int sockaddr_size = sizeof(SOCKADDR_IN);
	SOCKADDR_IN clientsockaddr;
	SOCKET clientRecvSocket = INVALID_SOCKET;
	SOCKET clientSendSocket = INVALID_SOCKET;
	PPerSocketContext pPerSocketCtx = NULL;
	ClientObject* pClientObj = NULL;
	m_bChkAcceptStop = false;

	while (TRUE)
	{
		if (m_bChkAcceptStop)
			break;
		//clientRecvSocket = INVALID_SOCKET;
		clientRecvSocket = accept(m_listenSocket, (LPSOCKADDR)&clientsockaddr, &sockaddr_size);

		if (clientRecvSocket == INVALID_SOCKET)
		{
			// 리슨 소켓을 클로즈 하면 이 에러가 나오므로
			// 이 에러시에 Accept 루프를 빠져나간다.
			if (WSAGetLastError() == WSAEINTR)
			{
				return;
			}
			U_Log("COMMON", "[ERROR]Accepting Client Error: %d ", WSAGetLastError());
		}

		DuplicateHandle(GetCurrentProcess(), (HANDLE)clientRecvSocket, GetCurrentProcess(), (LPHANDLE)&clientSendSocket, NULL, FALSE, DUPLICATE_SAME_ACCESS);

		pClientObj = AddClientObject(clientsockaddr);

		//	pClientObj->GetVCIBolckInfo();

		//PPerSocketContext pPerSocketCtx = NULL;
		pPerSocketCtx = new PerSocketContext;
		pPerSocketCtx->recvContext = CreateIOCtx();
		pPerSocketCtx->sendContext = CreateIOCtx();
		pPerSocketCtx->recvSocket = clientRecvSocket;
		pPerSocketCtx->sendSocket = clientSendSocket;
		CString strTmp;
		strTmp.Format("%s", inet_ntoa(clientsockaddr.sin_addr));

		memcpy(pPerSocketCtx->strClientIP, inet_ntoa(clientsockaddr.sin_addr), SIZE_IP);
		int nIndex = strTmp.ReverseFind('.');
		CString strTmp2 = strTmp.Mid(nIndex+1, strTmp.GetLength());
		int nTmp = atoi(strTmp2);
		pPerSocketCtx->ClientObjNo = nTmp;


		pClientObj->SetPerSocketCtx(pPerSocketCtx);
		// IOCP 커널 객체와 연결 //  completion key 생성
		if (!m_IocpHandler.Associate(clientRecvSocket, reinterpret_cast<ULONG_PTR>(pPerSocketCtx), &ErrCode))
		{
			////TRACE("Associating Error: %d\n",ErrCode);
			continue;
		}

		bool bRet = RecvPost(pPerSocketCtx);
		if (!bRet)
		{
			////TRACE("Init RecvPost Error\n");
			continue;
		}

		pMainWnd = AfxGetApp()->GetMainWnd();
		if (pMainWnd != NULL) pMainWnd->SendMessage(MSG_CONNECT, NULL, (LPARAM)(LPCSTR)pClientObj);
	
	}
}

// 완료 패킷 처리 함수
void NetworkController::ProcessingThread(void)
{
	ClientObject* pClientObj = NULL;
	PPerSocketContext pPerSocketCtx = NULL;
	PPerIoContext pPerIoCtx = NULL;
	DWORD dwBytesTransferred = 0;
	CString strClientIP;
	int ErrCode = 0;
	//	DWORD dObjID;
	while (TRUE)
	{
		// IO Completion Packet 얻어온다.
		BOOL bRet = m_IocpHandler.GetCompletionStatus(reinterpret_cast<ULONG_PTR*>(&pPerSocketCtx),
			&dwBytesTransferred,
			reinterpret_cast<LPOVERLAPPED*>(&pPerIoCtx),
			&ErrCode);
		//TRACE("1");
		
		if (bRet)
		{
			//TRACE("2");
			if (((int)pPerSocketCtx) == THREAD_DIE) //postqueue 를 이용한 처리 : all thread kill
			{
				//TRACE("3");
				break;
			}

			if (pPerIoCtx == NULL || (DWORD)pPerIoCtx == 0xfeeefeee)
			{
				//closeStep 1
				U_Log("COMMON", "[ERROR]Getting Completion Packet Failed %d\n", ErrCode);
				continue;
			}
		}
		else
		{
			if (NULL == pPerIoCtx)
			{
				U_Log("COMMON", "[ERROR]Getting Completion Packet Failed %d\n", ErrCode);
			}
			else
			{
				
				// 여기로 오면 에러가 64일 가능성이 높다.
				// 즉 지정된 네트워크 이름을 사용할 수 없습니다. 이다.
				// 뭐 이럴땐 소켓 끊어버리면 만사 OK이다.
				//pClientObj = (ClientObject*)dCompKey;
				//RemoveClientObj((LPCTSTR)(pPerSocketCtx->strClientIP));
				//strClientIP.Format("%s", pPerSocketCtx->strClientIP);
				//
				//if (strClientIP.GetLength() < 16){
				//	if (RemoveClientObj(strClientIP))
				//		U_Log("COMMON", "Client disconnect0");
				////		CloseSocketCtx(pPerSocketCtx);
				//}
				//
				U_Log("COMMON", "Client disconnect0");
				CloseSocketCtx(pPerSocketCtx); //clientObject 는 두고 socket만 제거
			}
			continue;
		}

		try
		{
			// 클라이언트가 연결 끊음 
			if (dwBytesTransferred == 0)
			{
				strClientIP.Format("%s", pPerSocketCtx->strClientIP);
				U_Log("COMMON", "Client disconnect %s", strClientIP);
				//RemoveClientObj((LPCTSTR)(pPerSocketCtx->strClientIP));
				//CloseSocketCtx(pPerSocketCtx);
				RemoveClientObj(strClientIP);
			//	CloseSocketCtx(pPerSocketCtx);
				continue;
			}
			else{
				if (pPerIoCtx == pPerSocketCtx->recvContext)
				{
					//TRACE("5");
					// RECV Operation 
					if (!RecvCompleteEvent(pPerSocketCtx, dwBytesTransferred))
					{
						throw "RecvCompleteEvent Error\n";
					}

				}
				else if (pPerIoCtx == pPerSocketCtx->sendContext)
				{
					//TRACE("6");
					// SEND Operation
					if (!SendCompleteEvent(pPerSocketCtx, dwBytesTransferred))
					{
						throw "SendCompleteEvent Error\n";
					}

				}
				else
				{
					//TRACE("7\n");
					if (!OtherCompleteEvent(pPerSocketCtx, dwBytesTransferred))
					{
						throw "OtherCompleteEvent Error\n";
					}
				}

			}


		}
		catch (char* errText)
		{

			//TRACE("8");
			U_SubLog("COMMON", (char*)(pPerSocketCtx->strClientIP), "[ERROR]processingThread %s", errText);
			RemoveClientObj(strClientIP);
		//	CloseSocketCtx(pPerSocketCtx);
		}

	}
}

void NetworkController::CloseSocketCtx(PPerSocketContext pPerSocketCtx)
{
	//if (pPerSocketCtx != NULL && (DWORD)pPerSocketCtx != 0xfeeefeee)
	//{
	//	Lock();
	//	shutdown(pPerSocketCtx->recvSocket, SD_BOTH);
	//	shutdown(pPerSocketCtx->sendSocket, SD_BOTH);
	//	closesocket(pPerSocketCtx->recvSocket);
	//	closesocket(pPerSocketCtx->sendSocket);
	//	//pPerSocketCtx->socket=INVALID_SOCKET;
	//	Sleep(100);
	//	if (pPerSocketCtx->recvSocket == INVALID_SOCKET){
	//		if (pPerSocketCtx->recvContext != NULL)
	//		{
	//			delete pPerSocketCtx->recvContext;
	//			pPerSocketCtx->recvContext = NULL;
	//		}
	//	}
	//	if (pPerSocketCtx->sendSocket == INVALID_SOCKET){
	//		if (pPerSocketCtx->sendContext != NULL)
	//		{
	//			delete pPerSocketCtx->sendContext;
	//			pPerSocketCtx->sendContext = NULL;
	//		}
	//	}
	////	delete pPerSocketCtx;
	//	
	////	pPerSocketCtx = NULL;
	//
	//
	//	
	//	//TRACE("\nCloseSocketCtx\n");
	//	unLock();
	//}
}

// ClientObject 생성 -> perSocketContext 생성 전달(completion key 로 쓰임)
ClientObject* NetworkController::AddClientObject(SOCKADDR_IN clientSocketAddr)
{
	//EnterCriticalSection(&m_cs);
	Lock();
	//PPerSocketContext pPerSocketCtx = NULL;
	ClientObject* pClientObject = NULL;


	char * strIP = inet_ntoa(clientSocketAddr.sin_addr);
	pClientObject = FindClientObject(strIP);

	CString strTmp;
	strTmp.Format("%s", strIP);
	//int nTmp = atoi(strTmp.Right(3));

	strTmp.Format("%s", strIP);
	int nIndex = strTmp.ReverseFind('.');
	CString strTmp2 = strTmp.Mid(nIndex + 1, strTmp.GetLength());
	int nTmp = atoi(strTmp2);

	if (pClientObject != NULL)
	{
		U_Log("COMMON", "  Create Client Object [IP : %s] [%d]", strIP, nTmp);
		U_SubLog("COMMON", strIP, " [CREATE Client Object ID:%d]", pClientObject);
		RemoveClientObj(strIP);
	}

	//if (pClientObject == NULL)
	//{
		pClientObject = new ClientObject();
		TRACE("\n\r CreateObject");
		pClientObject->m_strIP = strIP;
		memset(pClientObject->m_cstrIP, 0x00, sizeof(pClientObject->m_cstrIP));
		strcpy_s(pClientObject->m_cstrIP, strIP);


		pClientObject->m_nConnectType = CONNECT_TYPE::NEW_CONNECT;
		m_Objects.insert(hash_map<int, ClientObject*>::value_type(nTmp, pClientObject));
		pClientObject->SetCallBackForSend(NetworkController::SendPost);

		//pClientObject->SetSharedMemoryPosition();


		
	//}
	//else{

	//	pClientObject->m_nConnectType = CONNECT_TYPE::RE_CONNECT;// 다시 붙었을경우
	//	U_SubLog("COMMON", strIP, "*****************************[Socket ReConnect]");
	//	U_Log("COMMON", " [Socket ReConnect]EXiST Client Object [IP : %s]", strIP);
	//	//return NULL;
	//}
	//LeaveCriticalSection(&m_cs);
	unLock();
	return pClientObject;
}

bool NetworkController::RemoveClientObj(CString strIP)
{
	//int nSeqVCI = 0;
	bool bTmp = false;
	Lock();
	
	ClientObject* pClientObj = FindClientObject(strIP);

	if (pClientObj != NULL){
		U_SubLog("COMMON", (LPSTR)(LPCTSTR)strIP, "[REMOVE Client]");
		U_Log("COMMON", "  Delete Client Object [IP : %s]", strIP);

		delete pClientObj;
		CString strTmp;
		strTmp.Format("%s", strIP);
		int nIndex = strTmp.ReverseFind('.');
		CString strTmp2 = strTmp.Mid(nIndex + 1, strTmp.GetLength());
		int nTmp = atoi(strTmp2);

		m_Objects.erase(nTmp);
		bTmp = true;
	}
	unLock();
	return bTmp;

	

}
void NetworkController::RemoveAllClientObject()
{
	//	EnterCriticalSection(&m_cs);
	Lock();
	//ClientObject* pClientObj;
	//hash_map<int, ClientObject*>::iterator  Iter;
	int  nCnt = m_Objects.size();
	for (int i = 0; i<nCnt; i++)
	{
		RemoveClientObj(m_Objects.begin()->second->m_strIP);
		Sleep(200);
		U_Log("COMMON", "*****************RemoveClientObj %d/%d", i, nCnt);
	}
	unLock();
	/*for(Iter = m_Objects.begin(); Iter != m_Objects.end(); ++Iter)
	{
	RemoveClientObj(Iter->second->m_strIP);
	Sleep(200);
	}*/


	//	LeaveCriticalSection(&m_cs);
}

// RECV 요청
bool NetworkController::RecvPost(PPerSocketContext pPerSocketCtx)
{
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	//PPerIoContext pPerIOCtx = pClientObj->GetPerIOCtxForRecv();

	Lock();
	memset(pPerSocketCtx->recvContext->Buffer, 0x00, MAX_BUFFER);
	ZeroMemory(&pPerSocketCtx->recvContext->overlapped, sizeof(WSAOVERLAPPED));

	if (pPerSocketCtx->recvSocket == INVALID_SOCKET)
	{
		U_Log("COMMON", "[ERROR] [%s]INVALID_SOCKET.\n", (LPSTR)(LPCTSTR)(pPerSocketCtx->strClientIP));
		return false;
	}

	U_SubLog("COMMON", (LPSTR)(LPCTSTR)(pPerSocketCtx->strClientIP), "[RECV] Call Function RecvPost()");
	int ret = WSARecv(pPerSocketCtx->recvSocket, &(pPerSocketCtx->recvContext->wsaBuf), 1,
		&dwRecvBytes, &dwFlags, &(pPerSocketCtx->recvContext->overlapped), NULL);
	unLock();

	if (SOCKET_ERROR == ret)
	{
		int ErrCode = WSAGetLastError();
		if (ErrCode != WSA_IO_PENDING)
		{
			//U_SubLog("COMMON",(LPSTR)(LPCTSTR)(pClientObj->GetClientIP()), "[ERROR][%d] Recv Request Error(WSASend Function Failed): %d",GetTickCount(),ErrCode);
			U_Log("COMMON", "[%d][ERROR] [%s]Send Request Error(WSASend Function Failed): %d\n", GetTickCount(), (LPSTR)(LPCTSTR)(pPerSocketCtx->strClientIP), ErrCode);
			U_Log("COMMON", "[%d][ERROR] [%s]ClientSocket will Close.\n", GetTickCount(), (LPSTR)(LPCTSTR)(pPerSocketCtx->strClientIP));
			return FALSE;
		}
	}
	else{

	}
	return TRUE;
}

// Send 요청
bool NetworkController::SendPost(PPerSocketContext pPerSocketCtx)
{
	//EnterCriticalSection(&m_cs);
	//ClientObject* pClientObj = (ClientObject*)pClientObj_ ;
	//Lock();
	DWORD dwSendBytes = 0;
	DWORD dwFlags = 0;
	ZeroMemory(&pPerSocketCtx->sendContext->overlapped, sizeof(WSAOVERLAPPED));
	PBASIC_PACKET pPacket = (PBASIC_PACKET)(pPerSocketCtx->sendContext->Buffer);

	if (pPacket->cCmdID != CMDID::FID_PV_SENDFILESTREAM)
	{
		char cBuf[MAX_BUFFER];
		CString temp1, temp2;
		DWORD nLen = 0;

		nLen += ((DWORD)(pPacket->nLength[0] & 0xff)) << 8;
		nLen += ((DWORD)(pPacket->nLength[1] & 0xff));
		nLen += SIZE_HEADER_REAR;
		memcpy(&cBuf, pPacket, nLen);

		for (DWORD i = 0; i<nLen; i++){
			temp1.Format("%s %02x", temp2, cBuf[i] & 0xff);
			temp2 = temp1;
		}
		U_SubLog("PROTOCOL", (LPSTR)(pPerSocketCtx->strClientIP), "SEND: %d Bytes -> %s", nLen, temp2);
	}
	else{
		char cBuf[MAX_BUFFER];
		CString temp1, temp2;
		DWORD nLen = 0;

		nLen += ((DWORD)(pPacket->nLength[0] & 0xff)) << 8;
		nLen += ((DWORD)(pPacket->nLength[1] & 0xff));
		nLen += SIZE_HEADER_REAR;
		memcpy(&cBuf, pPacket, nLen);

		int nTmp = 0;
		if (nLen > 100) nTmp = 100;

		for (int i = 0; i<nTmp; i++){
			temp1.Format("%s %02x", temp2, cBuf[i] & 0xff);
			temp2 = temp1;
		}

		U_SubLog("PROTOCOL", (LPSTR)(pPerSocketCtx->strClientIP), "SEND: %d Bytes -> %s", nLen, temp2);

	}
	if (pPerSocketCtx->sendSocket == INVALID_SOCKET)
	{
		U_Log("COMMON", "\n [ERROR] SendSocket is INVALID_SOCKET ");
		U_Log("COMMON", "Client will Close.\n");
		return false;
	}

	int ret = WSASend(pPerSocketCtx->sendSocket, &(pPerSocketCtx->sendContext->wsaBuf), 1,
		&dwSendBytes, dwFlags, &(pPerSocketCtx->sendContext->overlapped), NULL);

	if (SOCKET_ERROR == ret)
	{
		int ErrCode = WSAGetLastError();
		if (ErrCode != WSA_IO_PENDING)
		{
			U_Log("COMMON", "[ERROR][%d] Send Request Error(WSASend Function Failed): %d\n", GetTickCount(), ErrCode);
			U_Log("COMMON", "[ERROR]Client will Close.\n");
			return FALSE;
		}
	}


	//unLock();
	return TRUE;
}
// 리시브 이벤트 처리 핸들러 함수
// return 값: TRUE  -> 에러 없이 정상적으로 처리됨
//            FALSE -> 완료 패킷 처리 동작 중 에러 발생
bool NetworkController::RecvCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred)
{
	//Lock();
	CString temp1, temp2;
	ClientObject* pClientObj;
	PPerIoContext pPerIoCtx = (PPerIoContext)(pPerSocketCtx->recvContext);

	CString strIp;
	strIp.Format("%s", pPerSocketCtx->strClientIP);
	pClientObj = FindClientObject(strIp);
	PBASIC_PACKET pBasic_packet = (PBASIC_PACKET)pPerIoCtx->Buffer;

	U_SubLog("COMMON", (LPSTR)(LPCTSTR)(pPerSocketCtx->strClientIP), "[RECV] Call Function RecvCompleteEvent()");
	DWORD nLength = 0;
	int nResult = 0;
	int nCnt = 0;

	if (pBasic_packet->cSource != TERMINALID::DEVICE){
		if (pClientObj->m_nPacketCurrentSize > 0){
			memcpy(&pClientObj->cPacketBuf[pClientObj->m_nPacketCurrentSize], pPerIoCtx->Buffer, dwBytesTransferred);
			pClientObj->m_nPacketCurrentSize += dwBytesTransferred;
		}

	}
	else if (pBasic_packet->cSource == TERMINALID::DEVICE && pBasic_packet->cDestination == TERMINALID::SERVER)
	{
		pClientObj->m_nPacketCurrentSize = 0;
		memset(pClientObj->cPacketBuf, 0x00, MAX_BUFFER);
		memcpy(pClientObj->cPacketBuf, pPerIoCtx->Buffer, dwBytesTransferred);
		pClientObj->m_nPacketCurrentSize += dwBytesTransferred;
		pClientObj->m_nPacketAllSize = 0;
		pClientObj->m_nPacketAllSize = ((int)(pClientObj->cPacketBuf[3] & 0xff)) << 8;
		pClientObj->m_nPacketAllSize += (int)(pClientObj->cPacketBuf[4] & 0xff);
		pClientObj->m_nPacketAllSize += SIZE_HEADER_REAR;
	}
	if (pClientObj->m_nPacketAllSize == pClientObj->m_nPacketCurrentSize && pClientObj->m_nPacketCurrentSize > 0)
	{
		pBasic_packet = (PBASIC_PACKET)pClientObj->cPacketBuf;
		//U_SubLog("COMMON",(LPSTR)(pPerSocketCtx->strClientIP),"******************************* %d : %d*****************************",pClientObj->m_nPacketAllSize,pClientObj->m_nPacketCurrentSize);
	
		DWORD nCnt = 0;
		CString temp1, temp2;
		if (dwBytesTransferred>1000)
			nCnt = 1000;
		else
			nCnt = dwBytesTransferred;

		for (DWORD i = 0; i<nCnt; i++){
			temp1.Format("%s %02x", temp2, pClientObj->cPacketBuf[i] & 0xff);
			temp2 = temp1;
		}
		U_SubLog("PROTOCOL", (LPSTR)(pPerSocketCtx->strClientIP), "RECV: %d Bytes -> %s", nCnt, temp2);

		nResult = pClientObj->ProcessPacket();
	}
	else
	{
		/*	DWORD nCnt = 0;
		CString temp1, temp2;
		if(dwBytesTransferred>500)
		nCnt = 500;
		else
		nCnt = dwBytesTransferred;

		for(DWORD i=0; i<nCnt;i++){
		temp1.Format("%s %02x", temp2,pPerSocketCtx->recvContext->wsaBuf.buf[i]&0xff);
		temp2 = temp1;
		}*/
		//if(pBasic_packet->cCmdID != CMDID::FID_PV_READFILE)
		//	U_SubLog("COMMON",(LPSTR)(pPerSocketCtx->strClientIP),"RECV_not all(all:%d): %d Bytes",pClientObj->m_nPacketCurrentSize,dwBytesTransferred);

	}


	//한번더 연결 해주기 IOCP 특징
	memset(pPerSocketCtx->recvContext->Buffer, 0x00, MAX_BUFFER);
	bool bRet = RecvPost(pPerSocketCtx);
	//unLock();
	if (!bRet)
		return false;

	return true;
}

// Send 완료 패킷 처리 핸들러 함수
// return 값: TRUE  -> 에러 없이 정상적으로 처리됨
//            FALSE -> 완료 패킷 처리 동작 중 에러 발생
bool NetworkController::SendCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred)
{
	return true;
}

// Recv, Send 완료 동작 외의 처리 핸들러 함수
// return 값: TRUE  -> 에러 없이 정상적으로 처리됨
//            FALSE -> 완료 패킷 처리 동작 중 에러 발생
bool NetworkController::OtherCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred)
{
	U_Log("COMMON", "[ERROR]Critical Error. Invalid Operation. Client Close!\n");
	return false; // 에러를 가라킴
}

// 서버 중단
void NetworkController::ServerClose(void)
{

	m_bChkAcceptStop = true;
	if (m_listenSocket != INVALID_SOCKET)
	{
		shutdown(m_listenSocket, SD_BOTH);
		closesocket(m_listenSocket);
		m_listenSocket = INVALID_SOCKET;
		U_Log("COMMON", "Close Socket");

		RemoveAllClientObject();
		m_IocpHandler.CloseAllThreads(); // processThread 종료
		CWnd* pWnd = AfxGetApp()->GetMainWnd();
		if (pWnd != NULL) pWnd->SendMessage(MSG_SOCKETCLOSE, NULL, NULL);

	}
}

ClientObject* NetworkController::FindClientObject(CString strIP)
{
	ClientObject* pClientObject = NULL;
	CString strTmp;
	strTmp.Format("%s", strIP);
	int nIndex = strTmp.ReverseFind('.');
	CString strTmp2 = strTmp.Mid(nIndex + 1, strTmp.GetLength());
	int nClientObj = atoi(strTmp2);

	if (m_Objects.size()>0){
		hash_map<int, ClientObject*>::iterator FindIter = m_Objects.find(nClientObj);
		if (m_Objects.end() != FindIter)
		{
			pClientObject = FindIter->second;
			return pClientObject;
		}
	}
	return pClientObject;

}
void NetworkController::Lock(){
	WaitForSingleObject(hMutex, INFINITE);
}
void NetworkController::unLock(){
	ReleaseMutex(hMutex);
}

PPerIoContext NetworkController::CreateIOCtx()
{
	PPerIoContext pPerIoCtx = new PerIoContext;
	pPerIoCtx->wsaBuf.buf = pPerIoCtx->Buffer;
	pPerIoCtx->wsaBuf.len = MAX_BUFFER;

	return pPerIoCtx;
}

