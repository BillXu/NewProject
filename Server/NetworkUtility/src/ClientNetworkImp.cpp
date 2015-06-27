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
	stConnectMission* cc = new stConnectMission ;
	memset(cc->cIP,0,sizeof(cc->cIP)) ;
	sprintf_s(cc->cIP,sizeof(cc->cIP),"%s",pIP);
	cc->nConnectID = ++nServerID ;
	cc->nPort = nPort ;
	CAutoLock cl (m_lockConnectMission);
	m_vWillConnect.push_back(cc);
	return nServerID ;
}

void CClientNetwork::Run()
{
	while (m_bRunning )
	{
		DoConnectMission();
		m_ios->RunOnce();
		ProcessSendMsg();
		CheckHeatBeat();
	}
	// clean all data 
	MAP_ID_SESSION::iterator iter = m_vAllSessions.begin() ;
	for ( ; iter != m_vAllSessions.end() ; ++iter )
	{
		iter->second->OnConnect(false); // this method will detete tcpsocket and it self ;
	}
	m_vAllSessions.clear() ;

	delete this ;
}

void CClientNetwork::DoConnectMission()
{
	VEC_CONNECT_MISSION vWill ;
	vWill.clear();
	{
		CAutoLock cl (m_lockConnectMission) ;
		vWill.swap(m_vWillConnect);
	}
	
	for ( unsigned short nIdx = 0 ; nIdx < vWill.size(); ++nIdx )
	{
		stConnectMission* pcc = vWill[nIdx];
		CSession* pSe = new CSession ;
		bool bRet = pSe->InitSocketForClient(this,m_ios,pcc->cIP,pcc->nPort,pcc->nConnectID) ;
		if ( !bRet )
		{
			delete pSe ;
			pSe = NULL ;
		}
		delete pcc;
		pcc = NULL;
	}
	vWill.clear() ;
}