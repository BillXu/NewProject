#pragma once
#include "ISeverApp.h"
#include "Singleton.h"
#include "RoomConfig.h"
class CGoldenServerApp
	:public IServerApp
	,public CSingleton<CGoldenServerApp>
{
public:
	CGoldenServerApp(){}
	bool init()override;
	uint16_t getLocalSvrMsgPortType() override ;
	CRoomConfigMgr* getRoomConfigMgr(){ return &m_tMgr ;}
protected:
	CRoomConfigMgr m_tMgr ;
};