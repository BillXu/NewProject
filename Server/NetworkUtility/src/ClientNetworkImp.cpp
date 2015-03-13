#include "ClientNetworkImp.h"
#include "Session.h"
void CClientNetwork::Start()
{
	m_ios = CreateIOServer();
	if (!m_ios->Initialize(this))
	{
		LOGE("manager start fail!");
		return;
	}
	INetwork::Start();
}

CONNECT_ID CClientNetwork::ConnectToServer(const char* pIP, unsigned short nPort )
{
	static CONNECT_ID nServerID = 1 ;
	CSession* pSe = new CSession ;
	bool bRet = pSe->InitSocketForClient(this,m_ios,pIP,nPort,++nServerID) ;
	if ( !bRet )
	{
		delete pSe ;
		pSe = NULL ;
	}
	return bRet ? nServerID : INVALID_CONNECT_ID;
}