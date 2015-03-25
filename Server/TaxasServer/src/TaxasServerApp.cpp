#include "TaxasServerApp.h"
#include "CommonDefine.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
CTaxasServerApp* CTaxasServerApp::s_TaxasServerApp = NULL ;
CTaxasServerApp* CTaxasServerApp::SharedGameServerApp()
{
	return s_TaxasServerApp ;
}

CTaxasServerApp::~CTaxasServerApp()
{
	delete m_pTimerMgr ;
	delete m_pNetWork ;
	delete m_pRoomConfig ;
}

void CTaxasServerApp::Init()
{
	m_bRunning = true ;
	if ( s_TaxasServerApp == NULL )
	{
		s_TaxasServerApp = this ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Taxas Server App can not be init more than once !") ;
		return ;
	}

	srand((unsigned int)time(0));
	m_nCenterSvrNetworkID = INVALID_CONNECT_ID ;
	
	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
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

	m_pRoomConfig = new CRoomConfigMgr ;
	m_pRoomConfig->LoadFile("../configFile/RoomConfig.txt") ;
	// init component ;
	m_pTimerMgr = new CTimerManager ;
}

bool CTaxasServerApp::OnMessage( Packet* pMsg )
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

	CLogMgr::SharedLogMgr()->ErrorLog("unprocess msg = %d , from port = %d , nsssionid = %d",preal->usMsgType,pData->nSenderPort,pData->nSessionID ) ;
	return true ;
}

bool CTaxasServerApp::ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	return false ;
}

bool CTaxasServerApp::OnLostSever(Packet* pMsg)
{
	m_nCenterSvrNetworkID = INVALID_CONNECT_ID ;
	return false ;
}

bool CTaxasServerApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	if ( eConnect_Accepted == eSate )
	{
		m_nCenterSvrNetworkID = pMsg->_connectID ;
		stMsg cMsg ;
		cMsg.cSysIdentifer = ID_MSG_PORT_CENTER ;
		cMsg.usMsgType = MSG_VERIFY_TAXAS ;
		m_pNetWork->SendMsg((char*)&cMsg,sizeof(stMsg),pMsg->_connectID) ;
		CLogMgr::SharedLogMgr()->SystemLog("Connected to Center Svr") ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("connect Center svr failed") ;
	return false;
}

bool CTaxasServerApp::Run()
{
	while (m_bRunning )
	{
		m_pNetWork->ReciveMessage() ;
		m_pTimerMgr->Update() ;
		Sleep(6);
	}
	ShutDown() ;
	return true ;
}

void CTaxasServerApp::ShutDown()
{
	m_pNetWork->ShutDown() ;
}

bool CTaxasServerApp::SendMsg(  unsigned int nSessionID , const char* pBuffer , int nLen, bool bBroadcast )
{
	if ( m_pNetWork == NULL || m_nCenterSvrNetworkID == INVALID_CONNECT_ID  )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr is not connected can not send msg") ;
		return false ;
	}

	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = ID_MSG_PORT_TAXAS ;
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
