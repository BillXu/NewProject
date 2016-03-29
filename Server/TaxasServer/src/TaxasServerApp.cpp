#include "TaxasServerApp.h"
#include "CommonDefine.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
#include "RoomManager.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
CTaxasServerApp* CTaxasServerApp::s_TaxasServerApp = NULL ;
CTaxasServerApp* CTaxasServerApp::SharedGameServerApp()
{
	return s_TaxasServerApp ;
}

CTaxasServerApp::~CTaxasServerApp()
{
	delete m_pRoomConfig ;
}

CTaxasServerApp::CTaxasServerApp()
{
	 m_pRoomConfig = NULL;
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
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	m_pRoomConfig = new CRoomConfigMgr ;
	m_pRoomConfig->LoadFile("../configFile/RoomConfig.txt") ;
	
	auto pp = new CRoomManager(m_pRoomConfig) ;
	registerModule(pp) ;
	return true ;
}
