#include "stdafx.h"
#include "ClientMng.h"

CClientMng::CClientMng()
{
}


CClientMng::~CClientMng()
{
}

// ClientObject 생성 -> perSocketContext 생성 전달(completion key 로 쓰임)
void CClientMng::CreateClientObject(SOCKET sock)
{
	Lock();
	ClientObject* pClientObject = NULL;
	pClientObject = FindClientObject(sock);
	if (pClientObject != NULL)
	{
		CString strIP = pClientObject->m_strIP;
		U_Log("COMMON", "  Create Client Object [IP : %s]", strIP);
		U_SubLog("COMMON", (LPSTR)(LPCTSTR)strIP, " [CREATE Client Object ID:%d]", pClientObject);
		RemoveClientObj(sock);
	}

	pClientObject = new ClientObject();
	//pClientObject->m_strIP = strIP;
	//memset(pClientObject->m_cstrIP, 0x00, sizeof(pClientObject->m_cstrIP));
	//strcpy_s(pClientObject->m_cstrIP, strIP);

	pClientObject->m_nConnectType = CONNECT_TYPE::NEW_CONNECT;
#ifdef USING_HASHMAP
	m_Objects.insert(hash_map<int, ClientObject*>::value_type(sock, pClientObject));
#else
	g_sClients.insert(pClientObject);
#endif

	unLock();
}

bool CClientMng::RemoveClientObj(SOCKET sock)
{
	Lock();
	bool bTmp = false;
	ClientObject* pClientObj = FindClientObject(sock);
	CString strIP = pClientObj->m_strIP;

	if (pClientObj != NULL){
		U_SubLog("COMMON", (LPSTR)(LPCTSTR)strIP, "[REMOVE Client]");
		U_Log("COMMON", "  Delete Client Object [IP : %s]", strIP);

		delete pClientObj;
		CString strTmp;
		strTmp.Format("%s", strIP);
		int nIndex = strTmp.ReverseFind('.');
		CString strTmp2 = strTmp.Mid(nIndex + 1, strTmp.GetLength());
		int nTmp = atoi(strTmp2);
#ifdef USING_HASHMAP
		m_Objects.erase(nTmp);
#else
		g_sClients.erase(pClientObj);
#endif
		bTmp = true;
	}
	unLock();
	return bTmp;
}

void CClientMng::RemoveAllClientObj()
{
	//Lock();
	//int  nCnt = m_Objects.size();
	//for (int i = 0; i<nCnt; i++)
	//{
	////	RemoveClientObj(m_Objects.begin()->second->);
	//	Sleep(200);
	//	U_Log("COMMON", "*****************RemoveClientObj %d/%d", i, nCnt);
	//}
	//unLock();
}

ClientObject* CClientMng::FindClientObject(SOCKET sock)
{
	ClientObject* pClientObject = NULL;

	for (CLIENT_SET::iterator it = g_sClients.begin(); it != g_sClients.end(); it++)
	{
		pClientObject = *it;
		if (pClientObject->m_sSock == sock){
			return pClientObject;
		}
	}

	return pClientObject;

}
void CClientMng::Lock(){
	WaitForSingleObject(hMutex, INFINITE);
}
void CClientMng::unLock(){
	ReleaseMutex(hMutex);
}

int CClientMng::GetHashKey(CString strIp){
	int nIndex = strIp.ReverseFind('.');
	CString strTmp2 = strIp.Mid(nIndex + 1, strIp.GetLength());
	int nTmp = atoi(strTmp2);

	return nTmp;
}