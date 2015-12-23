#include "NiuNiuServer.h"
#include "MessageDefine.h"
#include <ctime>
#include "LogManager.h"
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

	m_tMgr.LoadFile("../configFile/RoomConfig.txt") ;

	m_tRoomMgr.init();
	return true ;
}

uint16_t CNiuNiuServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_NIU_NIU ;
}

bool CNiuNiuServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( m_tRoomMgr.onMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	return false ;
}

void CNiuNiuServerApp::update(float fDeta )
{
	IServerApp::update(fDeta) ;
	m_tRoomMgr.update(fDeta) ;
}

void CNiuNiuServerApp::onExit()
{
	IServerApp::onExit();
	m_tRoomMgr.onTimeSave();
}