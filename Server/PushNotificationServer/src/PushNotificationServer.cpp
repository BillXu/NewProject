#include "PushNotificationServer.h"
#include "ServerMessageDefine.h"
#include "PushRequestQueue.h"
#include "LogManager.h"
CPushNotificationServer::~CPushNotificationServer()
{
	if ( m_pSvrNetWork )
	{
		m_pSvrNetWork->ShutDown();
		delete m_pSvrNetWork ;
		m_pSvrNetWork = NULL;
	}
}

bool CPushNotificationServer::Init()
{
	m_bRunning = true ;
	m_ServerConfigMgr.LoadFile("../configFile/serverConfig.txt") ;
	m_pSvrNetWork =  new CServerNetwork ;
	m_pSvrNetWork->StartupNetwork(m_ServerConfigMgr.GetServerConfig(eSvrType_APNS)->nPort,10,m_ServerConfigMgr.GetServerConfig(eSvrType_APNS)->strPassword) ;
	m_pSvrNetWork->AddDelegate(this) ;
	m_nPushThread.InitSSLContex();
	return m_nPushThread.ConnectToAPNs(); 
}

bool CPushNotificationServer::OnMessage( RakNet::Packet* pData )
{
	stMsg* pMsg = (stMsg*)pData->data ;
	if ( MSG_APNS_INFO == pMsg->usMsgType )
	{
		stMsgToAPNSServer* pRetMsg = (stMsgToAPNSServer*)pMsg;
		stNotice* pnotice = CPushRequestQueue::SharedPushRequestQueue()->GetReserverNotice() ;
		pnotice->nBadge = pRetMsg->nBadge ;
		memcpy(pnotice->pDeveiceToken,pRetMsg->pDeveiceToken,32); // must 32 ;
		char* pBuffer = (char*)pMsg ;
		pBuffer += sizeof(stMsgToAPNSServer);
		if ( pRetMsg->nAlertLen <= 0 || pRetMsg->nAlertLen >= 219 || pRetMsg->nSoundLen >= 50 )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Error notice , throw out ") ;
			delete pnotice ;
			pnotice = NULL ;
			return true ;
		}

		if ( pRetMsg->nSoundLen > 0  )
		{
			memcpy(pnotice->cSound,pBuffer,pRetMsg->nSoundLen);
			pBuffer += pRetMsg->nSoundLen ;
		}

		if ( pRetMsg->nAlertLen <= 0 )
		{
			delete pnotice ;
			pnotice = NULL ;
			CLogMgr::SharedLogMgr()->ErrorLog("Error: Alert content can not be null! ") ;
			return true ;
		}
		memcpy(pnotice->pAlert,pBuffer,pRetMsg->nAlertLen);
		CPushRequestQueue::SharedPushRequestQueue()->PushNotice(pnotice) ;
	}
	return true ;
}

void CPushNotificationServer::OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData )
{
	CLogMgr::SharedLogMgr()->PrintLog("a peer connected ip = %s\n",pData->systemAddress.ToString(true));
	stMsg msg ;
	msg.cSysIdentifer = ID_MSG_VERIFY ;
	msg.usMsgType = MSG_VERIFY_APNS ;
	m_pSvrNetWork->SendMsg((char*)&msg,sizeof(msg),nNewPeer,false) ;
}

void CPushNotificationServer::OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData )
{
	CLogMgr::SharedLogMgr()->PrintLog("a peer disconnected ip = %s\n",pData->systemAddress.ToString(true));
}

void CPushNotificationServer::Update()
{
	m_nPushThread.Start();
	while( m_bRunning )
	{
		if ( m_pSvrNetWork )
		{
			m_pSvrNetWork->RecieveMsg();
		}
		Sleep(20) ;
	}
	m_nPushThread.StopServer();
	m_pSvrNetWork->ShutDown();
}