#include "GateServer.h"
#include "CommonDefine.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "GateClient.h"
#define MAX_INCOME_PLAYER 5000
CGateServer* CGateServer::s_GateServer = NULL ;
CGateServer* CGateServer::SharedGateServer()
{
	return s_GateServer ;
}

CGateServer::CGateServer()
{
	m_bRunning = true ;
	m_pNetWorkForClients = NULL ;
	m_pNetWorkCenterSvr = NULL ;
	m_pGateManager = NULL ;
	m_nSvrIdx = m_nAllGeteCount = 0 ;
	m_nCurMaxSessionID = 0 ;
	m_nCenterServerNetID = INVALID_CONNECT_ID;
	if ( s_GateServer )
	{
		assert(0&&"only once should");
	}
}

CGateServer::~CGateServer()
{
	m_pNetWorkCenterSvr->ShutDown() ;
	if ( m_pNetWorkForClients )
	{
		m_pNetWorkForClients->ShutDown() ;
	}
	delete m_pNetWorkForClients ;
	delete m_pGateManager ;
	delete m_pNetWorkCenterSvr ;
	s_GateServer = NULL ;
}

void CGateServer::Init()
{
	if ( s_GateServer )
	{
		assert(0&&"only once should");
	}
	s_GateServer = this ;
	// client player mgr ;
	m_pGateManager = new CGateClientMgr ;
	
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");

	// connect to center svr ;
	ConnectToCenterServer();
}

void CGateServer::RunLoop()
{
	while ( m_bRunning )
	{
		if ( m_pNetWorkForClients )
		{
			m_pNetWorkForClients->RecieveMsg() ;
		}

		if ( m_pNetWorkCenterSvr )
		{
			m_pNetWorkCenterSvr->ReciveMessage();
		}

		if ( m_pGateManager )
		{
			m_pGateManager->UpdateReconectClientLife();
		}
		Sleep(2);
	}

	m_pNetWorkForClients->ShutDown() ;
	if ( IsCenterServerConnected() )
	{
		m_pNetWorkCenterSvr->DisconnectServer(m_nCenterServerNetID);
	}
}

void CGateServer::SendMsgToClient(const char* pData , int nLength , CONNECT_ID& nSendToOrExcpet ,bool bBroadcast )
{
	if ( m_pNetWorkForClients )
	{
		m_pNetWorkForClients->SendMsg(pData,nLength,nSendToOrExcpet,bBroadcast) ;
	}
}

void CGateServer::SendMsgToCenterServer( const char* pmsg, uint16_t nLen )
{
	if ( m_pNetWorkCenterSvr && IsCenterServerConnected() )
	{
		m_pNetWorkCenterSvr->SendMsg(pmsg,nLen) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center server is not connected ") ;
	}
}

bool CGateServer::OnMessage( Packet* pPacket )
{
	stMsg* pMsg = (stMsg*)pPacket->_orgdata ;
	if ( MSG_TRANSER_DATA == pMsg->usMsgType )
	{
		stMsgTransferData* pData = (stMsgTransferData*)pMsg;
		stMsg* pReal = (stMsg*)(pPacket->_orgdata + sizeof(stMsgTransferData));
		if ( pReal->cSysIdentifer == ID_MSG_PORT_CLIENT )
		{
			m_pGateManager->OnServerMsg((char*)pReal,pPacket->_len - sizeof(stMsgTransferData),pData->nSessionID ) ;
		}
		else if ( ID_MSG_PORT_GATE == pReal->cSysIdentifer )
		{
			OnMsgFromOtherSrvToGate(pReal,pData->nSenderPort);
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("wrong msg send to gate, target port = %d, msgType = %d",pReal->cSysIdentifer, pReal->usMsgType ) ;
		}
	}
	else if ( MSG_GATESERVER_INFO == pMsg->usMsgType )
	{
		stMsgGateServerInfo* pInfo = (stMsgGateServerInfo*)pMsg ;
		if ( pInfo->bIsGateFull )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("gate is full , can not setup more gate, plase colse this exe");
			m_pNetWorkForClients = NULL ;
			return true ;
		}
		m_nSvrIdx = pInfo->uIdx;
		m_nAllGeteCount = pInfo->uMaxGateSvrCount ;
		m_nCurMaxSessionID = m_nSvrIdx ;
		// start gate svr for client to connected 
		stServerConfig* pGateConfig = m_stSvrConfigMgr.GetGateServerConfig(m_nSvrIdx) ;
		m_pNetWorkForClients = new CServerNetwork ;
		m_pNetWorkForClients->StartupNetwork(pGateConfig->nPort,MAX_INCOME_PLAYER,pGateConfig->strPassword);
		m_pNetWorkForClients->AddDelegate(m_pGateManager) ;
		CLogMgr::SharedLogMgr()->SystemLog("setup network for clients to client ok " );
		CLogMgr::SharedLogMgr()->SystemLog("Gate Server Start ok idx = %d !",m_nSvrIdx ) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unknown msg = %d , from = %d",pMsg->usMsgType, pMsg->cSysIdentifer ) ;
	}

	return true ;
}

void CGateServer::OnMsgFromOtherSrvToGate( stMsg* pmsg , uint16_t eSendPort )
{

}

bool CGateServer::OnLostSever(Packet* pMsg)
{
	CLogMgr::SharedLogMgr()->ErrorLog("center server lost, we can not reconnect , please restart all gate svr ;");
	//m_pNetWorkCenterSvr->DisconnectServer(m_nCenterServerNetID);
	m_nCenterServerNetID = INVALID_CONNECT_ID ;
	//ConnectToCenterServer();
	return true ;
}

bool CGateServer::OnConnectStateChanged( eConnectState eSate, Packet* pMsg )                         
{
	if ( eConnect_Accepted == eSate )
	{
		m_nCenterServerNetID = pMsg->_connectID ;
		stMsg msg ;
		msg.cSysIdentifer = eSvrType_Center ;
		msg.usMsgType = MSG_VERIFY_GATE ;
		SendMsgToCenterServer((char*)&msg,sizeof(msg)) ;
		CLogMgr::SharedLogMgr()->SystemLog("connected center server");
	}
	else  // connect failed error 
	{
		CLogMgr::SharedLogMgr()->ErrorLog(" conencted failed, we can not reconnect , please restart all gate svr ;");
		//ConnectToCenterServer();
	}
	return true ;
}

void CGateServer::ConnectToCenterServer()
{
	stServerConfig* pSvrConfig = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pSvrConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("get eSvrType_Center config error " ) ;
		return ;
	}

	if ( NULL == m_pNetWorkCenterSvr )
	{
		m_pNetWorkCenterSvr = new CNetWorkMgr ;
		m_pNetWorkCenterSvr->SetupNetwork();
		m_pNetWorkCenterSvr->AddMessageDelegate(this);
	}
	m_pNetWorkCenterSvr->ConnectToServer(pSvrConfig->strIPAddress,pSvrConfig->nPort);
	CLogMgr::SharedLogMgr()->SystemLog("connecting to center server....");
}

uint32_t CGateServer::GenerateSessionID()
{
	return (m_nCurMaxSessionID += m_nAllGeteCount);
}

