#pragma once
#include "ISeverApp.h"
#include "PlayerManager.h"
#include "ConfigManager.h"
#include "ServerConfig.h"
#include "PokerCircle.h"
class CBrocaster ;
class CGameServerApp
	:public IServerApp
{
public:
	static CGameServerApp* SharedGameServerApp();
	~CGameServerApp();
	bool init();
	CPlayerManager* GetPlayerMgr(){ return m_pPlayerManager ; }
	CConfigManager* GetConfigMgr(){ return m_pConfigManager ; }
	bool onLogicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID);
	void update(float fdeta );
	uint16_t getLocalSvrMsgPortType(){ return ID_MSG_PORT_DATA ; } ; // et : ID_MSG_PORT_DATA , ID_MSG_PORT_TAXAS
	void onConnectedToSvr()override;
	void onExit()override;
protected:
	bool ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	void CheckNewDay();
public:
	static CGameServerApp* s_GameServerApp ;
protected:
	CPlayerManager* m_pPlayerManager ;
	CConfigManager* m_pConfigManager ;
	CPokerCircle m_tPokerCircle ;
	// check NewDay ;
	unsigned int m_nCurDay ;
};