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
	HANDLE hMutex; // thread �Ӱ迵�� ����ȭ Ŀ�θ��
	int m_nPort;
	SOCKET m_listenSocket;// ���� ����
#ifdef USING_HASHMAP
	//hash_map<int, ClientObject*> m_Objects;
#else
	CLIENT_SET g_sClients;
#endif
};

