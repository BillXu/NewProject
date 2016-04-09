#include "GoldenServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "LogManager.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
#include "GoldenRoomManager.h"
bool CGoldenServerApp::init()
{
	IServerApp::init();
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
	m_tMgr.LoadFile("../configFile/RoomConfig.txt") ;

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	auto* pMgr = new CGoldenRoomManager(&m_tMgr);
	registerModule(pMgr);
	return true ;
}

uint16_t CGoldenServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_GOLDEN ;
}

