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
	CNiuNiuServerApp(){}
	bool init()override;
	uint16_t getLocalSvrMsgPortType() override ;
	CRoomConfigMgr* getRoomConfigMgr(){ return &m_tMgr ;}
protected:
	CRoomConfigMgr m_tMgr ;
};