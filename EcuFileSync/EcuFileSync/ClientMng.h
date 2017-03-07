#pragma once
#include "ClientObject.h"
#include <hash_map>
#include <set>

using namespace std;

typedef set<ClientObject*> CLIENT_SET;
class CClientMng
{
public:
	CClientMng();
	~CClientMng();

	void Lock();
	void unLock();

	void CreateClientObject(SOCKET sock);
	ClientObject* FindClientObject(SOCKET sock);

	bool RemoveClientObj(SOCKET sock);
	void RemoveAllClientObj();

	int GetHashKey(CString strIp);

public:
	HANDLE hMutex; // thread 임계영역 동기화 커널모드
	int m_nPort;
	SOCKET m_listenSocket;// 리슨 소켓
#ifdef USING_HASHMAP
	//hash_map<int, ClientObject*> m_Objects;
#else
	CLIENT_SET g_sClients;
#endif
};

