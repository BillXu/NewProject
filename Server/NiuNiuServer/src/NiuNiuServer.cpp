#include "NiuNiuServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "LogManager.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
bool CNiuNiuServerApp::init()
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
	//m_tMgr.LoadFile("../configFile/RoomConfig.txt") ;

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	auto* pMgr = new CNiuNiuRoomManager(&m_tMgr);
	registerModule(pMgr);
	return true ;
}

uint16_t CNiuNiuServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_NIU_NIU ;
}

