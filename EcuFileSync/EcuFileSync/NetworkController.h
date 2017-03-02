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

	// �ʱ�ȭ ó��
	bool Init(const int nPort);
	// Accept �۾� ó��
	void AcceptProcess(void);
    // ���� �ߴ�
	void ServerClose(void);

	ClientObject* AddClientObject(SOCKADDR_IN clientSocketAddr);
	PPerIoContext CreateIOCtx();
	// Ŭ���̾�Ʈ ���� ���ؽ�Ʈ �����ϰ� ���� ����
	//void CloseClient(ClientObject* pClientObj);
	void CloseSocketCtx(PPerSocketContext pPerSocketCtx);
	//bool RemoveClientObject(ClientObject* pClientObj);
	bool RemoveClientObj(CString strIP);
	void RemoveAllClientObject();
	
	// �Ϸ� ��Ŷ ó�� �Լ�
	void ProcessingThread(void);
	// ���ú� �̺�Ʈ ó�� �ڵ鷯 �Լ�
	bool RecvCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred);
	// Send �Ϸ� ��Ŷ ó�� �ڵ鷯 �Լ�
	bool SendCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred);
	// Recv, Send �Ϸ� ���� ���� ó�� �ڵ鷯 �Լ�
	bool OtherCompleteEvent(PPerSocketContext pPerSocketCtx, DWORD dwBytesTransferred);
	
	// RECV ��û
	bool RecvPost(PPerSocketContext pPerSocketCtx);
	// Send ��û
	bool static SendPost(PPerSocketContext pPerSocketCtx);	
	
	ClientObject* FindClientObject(CString strIP);
	
	void Lock();
	void unLock();
	ClientObject* ResetHeartBitTime(CString strIP);
	//void HeartBitThreadRun(void);
public:
	CWnd* pMainWnd ;
	HANDLE hMutex; // thread �Ӱ迵�� ����ȭ Ŀ�θ��
	int m_nPort;
	SOCKET m_listenSocket;// ���� ����
	IocpHandler m_IocpHandler;// IOCP �ڵ鷯 
	hash_map<int, ClientObject*> m_Objects;
	bool m_bChkAcceptStop;
	int nTest ;
	
};
