#include "TaxasServerApp.h"
#include "CommonDefine.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
#include "RoomManager.h"
#include "ServerStringTable.h"
CTaxasServerApp* CTaxasServerApp::s_TaxasServerApp = NULL ;
CTaxasServerApp* CTaxasServerApp::SharedGameServerApp()
{
	return s_TaxasServerApp ;
}

CTaxasServerApp::~CTaxasServerApp()
{
	delete m_pRoomConfig ;
	delete m_pRoomMgr ;
}

CTaxasServerApp::CTaxasServerApp()
{
	 m_pRoomConfig = NULL;
	 m_pRoomMgr = NULL;
}

bool CTaxasServerApp::init()
{
	IServerApp::init();
	if ( s_TaxasServerApp == NULL )
	{
		s_TaxasServerApp = this ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Taxas Server App can not be init more than once !") ;
		return false;
	}

	srand((unsigned int)time(0));
	
	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");

	m_pRoomConfig = new CRoomConfigMgr ;
	m_pRoomConfig->LoadFile("../configFile/RoomConfig.txt") ;
	
	m_pRoomMgr = new CRoomManager ;
	m_pRoomMgr->init();
	return true ;
}

bool CTaxasServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( ProcessPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	if ( m_pRoomMgr && m_pRoomMgr->onMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("unprocess msg = %d , from port = %d , nsssionid = %d",prealMsg->usMsgType,eSenderPort,nSessionID ) ;
	return true ;
}

bool CTaxasServerApp::ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	return false ;
}

void CTaxasServerApp::onConnectedToSvr()
{
	if ( m_pRoomMgr )
	{
		m_pRoomMgr->onConnectedToSvr() ;
	}
}

void CTaxasServerApp::update(float fDeta )
{
	IServerApp::update(fDeta) ;
	m_pRoomMgr->update(fDeta);
}