#pragma once
#include "NetWorkManager.h"
#include "RoomManager.h"
#include "Timer.h"
#include "PlayerManager.h"
#include "ConfigManager.h"
#include "GameRank.h"
#include "ServerConfig.h"
class CBrocaster ;
class CGameServerApp
	:public CNetMessageDelegate
{
public:
	static CGameServerApp* SharedGameServerApp();
	~CGameServerApp();
	void Init();
	virtual bool OnMessage( RakNet::Packet* pMsg );
	virtual bool OnLostSever(RakNet::Packet* pMsg);
	bool OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg);
	bool Run();
	void ShutDown();

	void SendMsgToGateServer( unsigned int nSessionID , const char* pBuffer , int nLen, bool bBroadcast = false );
	void SendMsgToDBServer(const char* pBuffer , int nLen );
	void SendMsgToVerifyServer(const char* pBuffer, int nLen );
	void SendMsgToAPNsServer(const char* pBuffer, int nLen );
	CRoomManager* GetRoomMgr(){ return &m_pRoomMgr;}
	CTimerManager* GetTimerMgr(){ return m_pTimerMgr ; }
	CPlayerManager* GetPlayerMgr(){ return m_pPlayerManager ; }
	CConfigManager* GetConfigMgr(){ return m_pConfigManager ; }
	CGamerRank* GetGameRanker(){ return m_pGameRanker ;}
	CBrocaster* GetBrocaster(){ return m_pBrocaster ;}
	void Stop(){ m_bRunning = false ;}
protected:
	void ConnectToOtherServer();
	void OnDBServerConnected();
	bool ProcessGobalArgument(stMsg* pmsg );
	void RequestGlobalArgumentFromDB();
	void CheckNewDay();
	stServerConfig* GetServerConfig(eServerType eType);
	void SendMsgToSvr(eServerType eType ,const char* pBuffer , int nLen);
	bool IsSeverNeedToConnect(eServerType i){ return (i == eSvrType_Gate || eSvrType_DB == i || eSvrType_Verify == i /*|| eSvrType_APNS == i*/ || eSvrType_Log == i);}
public:
	static char* s_pBuffer ;
	static CGameServerApp* s_GameServerApp ;
protected:
	bool m_bRunning  ;
	// server associate 
	RakNet::RakNetGUID m_vSvrNetUID[eSvrType_Max] ;
	bool m_vIsSvrConnected[eSvrType_Max];
	// conpentent ;
	CRoomManager m_pRoomMgr ;
	CTimerManager* m_pTimerMgr ;
	CNetWorkMgr* m_pNetWork ;
	CPlayerManager* m_pPlayerManager ;

	CConfigManager* m_pConfigManager ;
	CGamerRank* m_pGameRanker ;
	CBrocaster* m_pBrocaster ;

	// server config 
	CSeverConfigMgr m_stSvrConfigMgr ;

	// check NewDay ;
	unsigned int m_nCurDay ;
};