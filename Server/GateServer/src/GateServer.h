#pragma once
#include "ServerNetwork.h"
#include "ClientManager.h"
#include "ServerConfig.h"
#include "NetWorkManager.h"
class CGateServer
	:public CNetMessageDelegate
{
public:
	static CGateServer* SharedGateServer();
	CGateServer();
	~CGateServer();
	void Init();
	CServerNetwork* GetNetWorkForClients(){ return m_pNetWorkForClients ;}
	void SendMsgToClient(const char* pData , int nLength , CONNECT_ID& nSendToOrExcpet ,bool bBroadcast = false );
	CGateClientMgr* GetClientMgr(){ return m_pGateManager ;}
	void RunLoop();
	void Stop(){ m_bRunning = false ;}
	void SendMsgToCenterServer(const char* pmsg, uint16_t nLen );

	// network
	bool OnMessage( Packet* pMsg );
	bool OnLostSever(Packet* pMsg);
	bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg );
	bool IsCenterServerConnected(){ return m_nCenterServerNetID != INVALID_CONNECT_ID ;}
	uint32_t GenerateSessionID();
protected:
	void OnMsgFromOtherSrvToGate(stMsg* pmsg, uint16_t eSendPort );
	void ConnectToCenterServer();
protected:
	static CGateServer* s_GateServer ;
	bool m_bRunning ;
	CServerNetwork* m_pNetWorkForClients ;
	CNetWorkMgr* m_pNetWorkCenterSvr ;
	CGateClientMgr* m_pGateManager ;

	CSeverConfigMgr m_stSvrConfigMgr ;
	uint16_t m_nSvrIdx ;  // value from Center svr ;
	uint16_t m_nAllGeteCount ;   // value from Center svr ;
	uint32_t m_nCurMaxSessionID ;
	CONNECT_ID m_nCenterServerNetID ;
};