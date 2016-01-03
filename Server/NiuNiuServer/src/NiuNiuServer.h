#pragma once
#include "ISeverApp.h"
#include "Singleton.h"
#include "RoomConfig.h"
#include "NiuNiuRoomManger.h"
class CNiuNiuServerApp
	:public IServerApp
	,public CSingleton<CNiuNiuServerApp>
{
public:
	bool init()override;
	uint16_t getLocalSvrMsgPortType() override ;
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID ) override;
	CRoomConfigMgr* getRoomConfigMgr(){ return &m_tMgr ;}
	CNiuNiuRoomManager* getRoomMgr(){ return &m_tRoomMgr ;}
	void update(float fDeta )override ;
	void onExit()override;
	void onConnectedToSvr()override;
protected:
	CRoomConfigMgr m_tMgr ;
	CNiuNiuRoomManager m_tRoomMgr ;
};