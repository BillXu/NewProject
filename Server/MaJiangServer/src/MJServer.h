#pragma once
#include "ISeverApp.h"
#include "Singleton.h"
#include "RoomConfig.h"
#include "MJServerRoomManger.h"
class CMJServerApp
	:public IServerApp
	,public CSingleton<CMJServerApp>
{
public:
	enum eModule
	{
		eMod_None = IServerApp::eDefMod_ChildDef,
		eMod_RoomMgr = eMod_None,
		eMod_RoomMgrOld,
		eMod_Max,
	};
public:
	CMJServerApp(){}
	bool init()override;
	uint16_t getLocalSvrMsgPortType() override ;
	CRoomConfigMgr* getRoomConfigMgr(){ return &m_tMgr ;}
	IGlobalModule* createModule(uint16_t eModuleType)override;
protected:
	CRoomConfigMgr m_tMgr ;
};