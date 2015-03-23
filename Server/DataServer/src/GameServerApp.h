#pragma once
#include "NetWorkManager.h"
#include "Timer.h"
#include "PlayerManager.h"
#include "ConfigManager.h"
#include "ServerConfig.h"
class CBrocaster ;
class CGameServerApp
	:public CNetMessageDelegate
{
public:
	static CGameServerApp* SharedGameServerApp();
	~CGameServerApp();
	void Init();
	virtual bool OnMessage( Packet* pMsg ) ;
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	bool Run();
	void ShutDown();
	bool SendMsg( unsigned int nSessionID , const char* pBuffer , int nLen, bool bBroadcast = false );
	CTimerManager* GetTimerMgr(){ return m_pTimerMgr ; }
	CPlayerManager* GetPlayerMgr(){ return m_pPlayerManager ; }
	CConfigManager* GetConfigMgr(){ return m_pConfigManager ; }
	void Stop(){ m_bRunning = false ;}
protected:
	bool ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	void CheckNewDay();
	stServerConfig* GetServerConfig(eServerType eType);
	bool IsSeverNeedToConnect(eServerType i){ return (i == eSvrType_Gate || eSvrType_DB == i || eSvrType_Verify == i /*|| eSvrType_APNS == i*/ || eSvrType_Log == i);}
public:
	static CGameServerApp* s_GameServerApp ;
protected:
	bool m_bRunning  ;
	// server associate 
	CONNECT_ID m_nCenterSvrNetworkID ;
	// conpentent ;
	CTimerManager* m_pTimerMgr ;
	CNetWorkMgr* m_pNetWork ;
	CPlayerManager* m_pPlayerManager ;

	CConfigManager* m_pConfigManager ;
	// server config 
	CSeverConfigMgr m_stSvrConfigMgr ;

	// check NewDay ;
	unsigned int m_nCurDay ;

	char m_pSendBuffer[MAX_MSG_BUFFER_LEN] ;
};