#pragma once

#include "IocpHandler.h"
#include "ProtocolDef.h"
#include "ClientObject.h"
#include <hash_map>
using namespace std;

class NetworkController:public IIocpProcessThread 
{
public:
	NetworkController(void);
	~NetworkController(void);

	// 초기화 처리
	bool Init(const int nPort);
	// Accept 작업 처리
	void AcceptProcess(void);
    // 서버 중단
	void ServerClose(void);

	ClientObject* AddClientObject(SOCKADDR_IN clientSocketAddr);
	PPerIoContext CreateIOCtx();
	// 클라이언트 소켓 컨텍스트 제거하고 소켓 닫음
	//void CloseClient(ClientObject* pClientObj);
	void CloseSocketCtx(PPerSocketContext pPerSocketCtx);
	//bool RemoveClientObject(ClientObject* pClientObj);
	bool RemoveClientObj(CString strIP);
	void RemoveAllClientObject();
	
	// 완료 패킷 처리 함수
	void ProcessingThread(void);
	// 리시브 이벤트 처리 핸들러 함수
	bool RecvCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred);
	// Send 완료 패킷 처리 핸들러 함수
	bool SendCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred);
	// Recv, Send 완료 동작 외의 처리 핸들러 함수
	bool OtherCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred);
	
	// RECV 요청
	bool RecvPost(PPerSocketContext pPerSocketCtx);
	// Send 요청
	bool static SendPost(PPerSocketContext pPerSocketCtx);	
	
	ClientObject* FindClientObject(CString strIP);
	
	void Lock();
	void unLock();
	ClientObject* ResetHeartBitTime(CString strIP);
	//void HeartBitThreadRun(void);
public:
	CWnd* pMainWnd ;
	HANDLE hMutex; // thread 임계영역 동기화 커널모드
	int m_nPort;
	SOCKET m_listenSocket;// 리슨 소켓
	IocpHandler m_IocpHandler;// IOCP 핸들러 
	hash_map<int, ClientObject*> m_Objects;
	bool m_bChkAcceptStop;
	int nTest ;
	
};
