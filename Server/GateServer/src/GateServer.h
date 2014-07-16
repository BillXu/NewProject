#pragma once
#include "ServerNetwork.h"
#include "ClientManager.h"
#include "ServerConfig.h"
class CTimerManager ;
class CGateServer
{
public:
	static CGateServer* SharedGateServer();
	CGateServer();
	~CGateServer();
	void Init();
	CServerNetwork* GetNetWork(){ return m_pNetWork ;}
	CGateClientMgr* GetClientMgr(){ return m_pGateManager ;}
	CTimerManager* GetTimerMgr(){ return m_pTimerManager ; }
	void RunLoop();
	void Stop(){ m_bRunning = false ;}
protected:
	static CGateServer* s_GateServer ;
	bool m_bRunning ;
	CServerNetwork* m_pNetWork ;
	CGateClientMgr* m_pGateManager ;
	CTimerManager* m_pTimerManager ;

	CSeverConfigMgr m_stSvrConfigMgr ;
};