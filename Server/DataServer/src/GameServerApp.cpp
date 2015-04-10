#include "GameServerApp.h"
#include "CommonDefine.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include <ctime>
#include "Timer.h"
#include "PlayerMail.h"
#include "EventCenter.h"
#ifndef USHORT_MAX
#define USHORT_MAX 65535 
#endif

CGameServerApp* CGameServerApp::s_GameServerApp = NULL ;
CGameServerApp* CGameServerApp::SharedGameServerApp()
{
	 
	return s_GameServerApp ;
}

CGameServerApp::~CGameServerApp()
{
	delete m_pTimerMgr ;
	delete m_pNetWork ;
	delete m_pPlayerManager ;
	delete m_pConfigManager ;
}

void CGameServerApp::Init()
{
	m_bRunning = true ;
	if ( s_GameServerApp == NULL )
	{
		s_GameServerApp = this ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Game Server App can not be init more than once !") ;
		return ;
	}

	srand((unsigned int)time(0));
	m_nCenterSvrNetworkID = INVALID_CONNECT_ID ;
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null , so can not connected to !") ;
		return ;
	}

	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(1);
	m_pNetWork->AddMessageDelegate(this);
	m_pNetWork->ConnectToServer(pConfig->strIPAddress,pConfig->nPort,pConfig->strPassword) ;
	CLogMgr::SharedLogMgr()->SystemLog("connecting to center svr ..") ;

	m_pConfigManager = new CConfigManager ;
	m_pConfigManager->LoadAllConfigFile("../configFile/") ;
	// init component ;
	m_pTimerMgr = new CTimerManager ;

	m_pPlayerManager = new CPlayerManager ;
	
	time_t tNow = time(NULL) ;
	m_nCurDay = localtime(&tNow)->tm_mday ;
}

bool CGameServerApp::OnMessage( Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->_len) ;
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		CLogMgr::SharedLogMgr()->SystemLog("no need recieve verify msg") ;
		return true ;
	}

	stMsg* pRet = (stMsg*)pMsg->_orgdata ;
	if ( pRet->usMsgType != MSG_TRANSER_DATA )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why msg type is not transfer data , type = %d",pRet->usMsgType ) ;
		return true;
	}

	stMsgTransferData* pData = (stMsgTransferData*)pRet ;
	stMsg* preal = (stMsg*)(pMsg->_orgdata + sizeof(stMsgTransferData));
	if ( ProcessPublicMsg(preal,(eMsgPort)pData->nSenderPort,pData->nSessionID) )
	{
		return true ;
	}

	if ( m_pPlayerManager->OnMessage(preal,(eMsgPort)pData->nSenderPort,pData->nSessionID) )
	{
		return true ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("unprocess msg = %d , from port = %d , nsssionid = %d",preal->usMsgType,pData->nSenderPort,pData->nSessionID ) ;
	return true ;
}

bool CGameServerApp::ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	return false ;
}

bool CGameServerApp::OnLostSever(Packet* pMsg)
{
	m_nCenterSvrNetworkID = INVALID_CONNECT_ID ;
	CLogMgr::SharedLogMgr()->ErrorLog("center server disconnected !") ;
	return false ;
}

bool CGameServerApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	if ( eConnect_Accepted == eSate )
	{
		m_nCenterSvrNetworkID = pMsg->_connectID ;
		stMsg cMsg ;
		cMsg.cSysIdentifer = ID_MSG_PORT_CENTER ;
		cMsg.usMsgType = MSG_VERIFY_DATA ;
		m_pNetWork->SendMsg((char*)&cMsg,sizeof(stMsg),pMsg->_connectID) ;
		CLogMgr::SharedLogMgr()->SystemLog("Connected to Center Svr") ;
		return false ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("connect Center svr failed") ;
	return false;
}

bool CGameServerApp::Run()
{
	while (m_bRunning )
	{
		m_pNetWork->ReciveMessage() ;
		m_pTimerMgr->Update() ;
		m_pPlayerManager->Update(0);	
		CheckNewDay();
		Sleep(6);
	}
	ShutDown() ;
	return true ;
}

void CGameServerApp::ShutDown()
{
	m_pNetWork->ShutDown() ;
}

bool CGameServerApp::SendMsg(  unsigned int nSessionID , const char* pBuffer , int nLen, bool bBroadcast )
{
	if ( m_pNetWork == NULL || m_nCenterSvrNetworkID == INVALID_CONNECT_ID  )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr is not connected can not send msg") ;
		return false ;
	}

	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = ID_MSG_PORT_DATA ;
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
	m_pNetWork->SendMsg(m_pSendBuffer,nLne,m_nCenterSvrNetworkID ) ;
	return true ;
}

void CGameServerApp::CheckNewDay()
{
	// check new day 
	time_t tNow = time(NULL) ;
	struct tm tmNow = *localtime(&tNow) ;
	if ( tmNow.tm_mday != m_nCurDay )
	{
		m_nCurDay = tmNow.tm_mday ;
		// new day 
		CEventCenter::SharedEventCenter()->PostEvent(eEvent_NewDay,&tmNow) ;
	}
}

stServerConfig* CGameServerApp::GetServerConfig(eServerType eType)
{
	stServerConfig* pDBConfig = m_stSvrConfigMgr.GetServerConfig(eType) ;
	if ( pDBConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find server config for Svr =%d",eType) ;
		return NULL ;
	}
	return pDBConfig ;
}