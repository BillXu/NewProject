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
	bool onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	CRoomConfigMgr* GetConfigMgr(){ return m_pRoomConfig ; }
	CRoomManager* GetRoomMgr(){ return m_pRoomMgr ; }
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_TAXAS ; }
	virtual void onConnectedToSvr();
	void update(float fDeta )override;
protected:
	bool ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
public:
	static CTaxasServerApp* s_TaxasServerApp ;
protected:
	CRoomConfigMgr* m_pRoomConfig;
	CRoomManager* m_pRoomMgr ;
};