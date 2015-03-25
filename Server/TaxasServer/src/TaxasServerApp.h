#pragma once
#include "NetWorkManager.h"
#include "Timer.h"
#include "RoomConfig.h"
#include "ServerConfig.h"
#include "MessageDefine.h"
class CTaxasServerApp
	:public CNetMessageDelegate
{
public:
	static CTaxasServerApp* SharedGameServerApp();
	~CTaxasServerApp();
	void Init();
	virtual bool OnMessage( Packet* pMsg ) ;
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	bool Run();
	void ShutDown();
	bool SendMsg( unsigned int nSessionID , const char* pBuffer , int nLen, bool bBroadcast = false );
	CTimerManager* GetTimerMgr(){ return m_pTimerMgr ; }
	CRoomConfigMgr* GetConfigMgr(){ return m_pRoomConfig ; }
	void Stop(){ m_bRunning = false ;}
protected:
	bool ProcessPublicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
public:
	static CTaxasServerApp* s_TaxasServerApp ;
protected:
	bool m_bRunning  ;
	// server associate 
	CONNECT_ID m_nCenterSvrNetworkID ;
	// conpentent ;
	CTimerManager* m_pTimerMgr ;
	CNetWorkMgr* m_pNetWork ;

	CRoomConfigMgr* m_pRoomConfig;

	char m_pSendBuffer[MAX_MSG_BUFFER_LEN] ;
};