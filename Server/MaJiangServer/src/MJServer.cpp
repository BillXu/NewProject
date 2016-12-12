#include "MJServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "log4z.h"
#include "ServerStringTable.h"
#include "RewardConfig.h"
#include "MJRoomManager.h"
bool CMJServerApp::init()
{
	IServerApp::init();
	srand((unsigned int)time(0));

	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		LOGFMTE("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);
	m_tMgr.LoadFile("../configFile/RoomConfig.txt") ;

	CServerStringTable::getInstance()->LoadFile("../configFile/stringTable.txt");
	CRewardConfig::getInstance()->LoadFile("../configFile/rewardConfig.txt");

	installModule(eMod_RoomMgr);
	installModule(eMod_RoomMgrOld);
	return true ;
}

uint16_t CMJServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_MJ ;
}

IGlobalModule* CMJServerApp::createModule(uint16_t eModuleType)
{
	auto p = IServerApp::createModule(eModuleType);
	if (p)
	{
		return p;
	}

	if (eModuleType == eMod_RoomMgrOld)
	{
		p = new CMJRoomManager(getRoomConfigMgr());
	}
	else if (eMod_RoomMgr == eModuleType)
	{
		p = new MJRoomManager();
	}
	return p;
}

