#include "GameServerApp.h"
#include "PlayerManager.h"
#include <ctime>
#include "PlayerMail.h"
#include "EventCenter.h"
#include "LogManager.h"
#include "RewardConfig.h"
#include "ServerStringTable.h"
#include "ExchangeCenter.h"
#include "RobotCenter.h"
#include "encryptNumber.h"
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
	delete m_pPlayerManager ;
	delete m_pConfigManager ;
}

bool CGameServerApp::init()
{
	IServerApp::init();
	if ( s_GameServerApp == NULL )
	{
		s_GameServerApp = this ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Game Server App can not be init more than once !") ;
		return false;
	}
	srand((unsigned int)time(0));

	CSeverConfigMgr SvrConfigMgr ;
	SvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = SvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	m_pConfigManager = new CConfigManager ;
	m_pConfigManager->LoadAllConfigFile("../configFile/") ;
	// init component ;
	m_pPlayerManager = new CPlayerManager ;

	auto pExc = new CExchangeCenter("../configFile/exchange.txt");
	registerModule(pExc);

	auto robotC = new CRobotCenter ;
	registerModule(robotC) ;

	auto pEncryptNumber = new CEncryptNumber();
	registerModule(pEncryptNumber) ;
	
	time_t tNow = time(NULL) ;
	m_nCurDay = localtime(&tNow)->tm_mday ;
	return true ;
}

bool CGameServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( IServerApp::onLogicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	if ( m_tPokerCircle.onMessage(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	if ( ProcessPublicMsg(prealMsg,eSenderPort,nSessionID ) )
	{
		return true ;
	}

	if ( m_pPlayerManager->OnMessage(prealMsg,eSenderPort,nSessionID ) )
	{
		return true ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("unprocess msg = %d , from port = %d , nsssionid = %d",prealMsg->usMsgType,eSenderPort,nSessionID ) ;
	return true ;
}

bool CGameServerApp::ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	return false ;
}


void CGameServerApp::update(float fdeta )
{
	IServerApp::update(fdeta);
	m_pPlayerManager->Update(fdeta);	
	CheckNewDay();
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

void CGameServerApp::onConnectedToSvr()
{
	IServerApp::onConnectedToSvr() ;
	m_tPokerCircle.readTopics();
}

void CGameServerApp::onExit()
{
	IServerApp::onExit() ;
	m_pPlayerManager->onExit() ;
}