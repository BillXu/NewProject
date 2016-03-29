#pragma once
#include "ISeverApp.h"
#include "RoomConfig.h"
#include "ServerConfig.h"
#include "MessageDefine.h"
class CRoomManager ;
class CTaxasServerApp
	:public IServerApp
{
public:
	static CTaxasServerApp* SharedGameServerApp();
	CTaxasServerApp();
	~CTaxasServerApp();
	bool init();
	CRoomConfigMgr* GetConfigMgr(){ return m_pRoomConfig ; }
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_TAXAS ; }
public:
	static CTaxasServerApp* s_TaxasServerApp ;
protected:
	CRoomConfigMgr* m_pRoomConfig;
};