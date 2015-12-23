#include <windows.h>
#include "ISeverApp.h"
#include "LogManager.h"
#include "MessageDefine.h"
#include "ServerMessageDefine.h"
#include <time.h>
#include <assert.h>
#include <synchapi.h>

#define TIME_WAIT_FOR_RECONNECT 5
bool IServerApp::init()
{
	m_bRunning = true;
	m_nTargetSvrNetworkID = INVALID_CONNECT_ID;
	
	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(1);
	m_pNetWork->AddMessageDelegate(this);

	m_pTimerMgr = new CTimerManager ;

	m_fReconnectTick = 0 ;
	return true ;
}

IServerApp::IServerApp()
{
	m_pTimerMgr = nullptr ;
	m_pNetWork = nullptr ;
	m_eConnectState = CNetWorkMgr::eConnectType_None ;
	memset(&m_stConnectConfig,0,sizeof(m_stConnectConfig));
	m_fReconnectTick = 0 ;
}

IServerApp::~IServerApp()
{
	if ( m_pTimerMgr )
	{
		delete m_pTimerMgr ;
		m_pTimerMgr = nullptr ;
	}

	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown() ;
		delete m_pNetWork ;
		m_pNetWork = nullptr ;
	}
}

bool IServerApp::OnMessage( Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->_len) ;
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		CLogMgr::SharedLogMgr()->SystemLog("no need recieve verify msg") ;
		return true ;
	}

	stMsg* pRet = pmsg;
	if ( pRet->usMsgType != MSG_TRANSER_DATA )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why msg type is not transfer data , type = %d",pRet->usMsgType ) ;
		return true;
	}

	stMsgTransferData* pData = (stMsgTransferData*)pRet ;
	stMsg* preal = (stMsg*)( pMsg->_orgdata + sizeof(stMsgTransferData));

	if ( onLogicMsg(preal,(eMsgPort)pData->nSenderPort,pData->nSessionID) )
	{
		return true ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("unprocessed msg = %d , from port = %d , session id = %d",preal->usMsgType,pData->nSenderPort,pData->nSessionID) ;
	return false ;
}

bool IServerApp::OnLostSever(Packet* pMsg)
{
	m_nTargetSvrNetworkID = INVALID_CONNECT_ID ;
	CLogMgr::SharedLogMgr()->ErrorLog("Target server disconnected !") ;
	m_eConnectState = CNetWorkMgr::eConnectType_Disconnectd ;
	return false ;
}

bool IServerApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	m_eConnectState = eConnect_Accepted == eSate ? CNetWorkMgr::eConnectType_Connected : CNetWorkMgr::eConnectType_Disconnectd ;
	if ( eConnect_Accepted == eSate )
	{
		m_nTargetSvrNetworkID = pMsg->_connectID ;
		stMsg cMsg ;
		cMsg.cSysIdentifer = getTargetSvrPortType() ;
		cMsg.usMsgType = getVerifyType() ;
		m_pNetWork->SendMsg((char*)&cMsg,sizeof(stMsg),pMsg->_connectID) ;
		CLogMgr::SharedLogMgr()->SystemLog("Connected to Target Svr") ;
		onConnectedToSvr();
		return false ;
	}

	m_nTargetSvrNetworkID = INVALID_CONNECT_ID ;
	CLogMgr::SharedLogMgr()->ErrorLog("connect target svr failed, %d seconds later reconnect",TIME_WAIT_FOR_RECONNECT) ;
	return false ;
}

bool IServerApp::run()
{
	clock_t t = clock();
	while ( m_bRunning )
	{
		if ( m_pNetWork )
		{
			m_pNetWork->ReciveMessage();
		}

		if ( m_pTimerMgr )
		{
			m_pTimerMgr->Update() ;
		}

		clock_t tNow = clock();
		float fDelta = float(tNow - t ) / CLOCKS_PER_SEC ;
		t = tNow ;
		update(fDelta);
		Sleep(10);
	}

	onExit();
	Sleep(4000);
	shutDown();
	return true ;
}

void IServerApp::shutDown()
{
	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown() ;
		m_eConnectState = CNetWorkMgr::eConnectType_None ;
	}
}

bool IServerApp::sendMsg( const char* pBuffer , int nLen )
{
	assert(m_pNetWork && "please invoke IServerApp init" );
	if ( isConnected() )
	{
		m_pNetWork->SendMsg(pBuffer,nLen,m_nTargetSvrNetworkID) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("target is disconnect , can not send msg");
	}
	return isConnected() ;
}

bool IServerApp::sendMsg(  uint32_t nSessionID , const char* pBuffer , uint16_t nLen, bool bBroadcast )
{
	if ( isConnected() == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("target svr is not connect , send msg failed") ;
		return false ;
	}
	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = getLocalSvrMsgPortType() ;
	msgTransData.bBroadCast = bBroadcast ;
	msgTransData.nSessionID = nSessionID ;
	int nLne = sizeof(msgTransData) ;
	if ( nLne + nLen >= MAX_MSG_BUFFER_LEN )
	{
		stMsg* pmsg = (stMsg*)pBuffer ;
		CLogMgr::SharedLogMgr()->ErrorLog("msg send to session id = %d , is too big , cannot send , msg id = %d ",nSessionID,pmsg->usMsgType) ;
		return false;
	}
	memcpy(m_pSendBuffer,&msgTransData,nLne);
	memcpy(m_pSendBuffer + nLne , pBuffer,nLen );
	nLne += nLen ;
	sendMsg(m_pSendBuffer,nLne);
	return true ;
}

void IServerApp::stop()
{
	m_bRunning = false ;
}

bool IServerApp::onLogicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	return false ;
}

void IServerApp::update(float fDeta )
{
	if ( m_eConnectState == CNetWorkMgr::eConnectType_Disconnectd )
	{
		m_fReconnectTick += fDeta ;
		if ( m_fReconnectTick >= TIME_WAIT_FOR_RECONNECT )
		{
			CLogMgr::SharedLogMgr()->SystemLog("Reconnecting....");
			doConnectToTargetSvr() ;
			m_fReconnectTick = 0 ;
		}
	}
}

uint16_t IServerApp::getTargetSvrPortType()
{
	return ID_MSG_PORT_CENTER ;
}

bool IServerApp::isConnected()
{
	return m_eConnectState == CNetWorkMgr::eConnectType_Connected;
}

void IServerApp::setConnectServerConfig(stServerConfig* pConfig )
{
	if ( pConfig == nullptr )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("connect config is null") ;
		return ;
	}

	m_stConnectConfig = *pConfig ;

	if ( m_eConnectState != CNetWorkMgr::eConnectType_Connected && CNetWorkMgr::eConnectType_Connecting != m_eConnectState )
	{
		doConnectToTargetSvr();
	}
}

void IServerApp::doConnectToTargetSvr()
{
	if ( m_eConnectState == CNetWorkMgr::eConnectType_Connecting || isConnected() )
	{
		return ;
	}

	assert(m_pNetWork && "IServer init not invoke" ) ;
	assert(m_stConnectConfig.nPort && "please set connect config" ) ;
	m_pNetWork->ConnectToServer(m_stConnectConfig.strIPAddress,m_stConnectConfig.nPort,m_stConnectConfig.strPassword) ;
	m_eConnectState = CNetWorkMgr::eConnectType_Connecting ;
	CLogMgr::SharedLogMgr()->SystemLog("connect to target svr ip = %s", m_stConnectConfig.strIPAddress );
}
uint16_t IServerApp::getVerifyType()
{
	switch ( getLocalSvrMsgPortType() )
	{
	case ID_MSG_PORT_CLIENT:
		return MSG_VERIFY_CLIENT ;
	case ID_MSG_PORT_GATE:
		return MSG_VERIFY_GATE ;
	case ID_MSG_PORT_LOGIN:
		return MSG_VERIFY_LOGIN ;
	case ID_MSG_PORT_VERIFY:
		return MSG_VERIFY_VERIYF ;
	case ID_MSG_PORT_APNS:
		return MSG_VERIFY_APNS ;
	case ID_MSG_PORT_LOG:
		return MSG_VERIFY_LOG; 
	case ID_MSG_PORT_DATA:
		return MSG_VERIFY_DATA ;
	case ID_MSG_PORT_TAXAS:
		return MSG_VERIFY_TAXAS;
	case ID_MSG_PORT_DB:
		return MSG_VERIFY_DB ;
	case ID_MSG_PORT_NIU_NIU:
		return MSG_VERIFY_NIU_NIU ;
	default:
		assert(0 && "what verify type for the svr ? " ) ;
		break;
	}
	return MSG_VERIFY_END ;
}