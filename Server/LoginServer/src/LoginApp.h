#pragma once 
#include "NetWorkManager.h"
#include "Timer.h"
#include "ServerConfig.h"
class CLoginPeerMgr;
class CLoginApp
	:public CTimerDelegate
	,public CNetMessageDelegate
{
public:
	struct stServerInfo 
	{
		bool m_bConnected ;
		RakNet::RakNetGUID m_nServerNetID ;
		std::string m_strIPAddress ;
		unsigned short m_nPort ;
	};

public:
	CLoginApp();
	~CLoginApp();
	void Init();
	void MainLoop();
	virtual bool OnMessage( RakNet::Packet* pMsg );
	virtual bool OnLostSever(RakNet::Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg);
	bool SendMsg( const char* pBuffer , unsigned int nLen , bool bGate );
	void ReconnectDB(float fTimeElaps,unsigned int nTimerID );
	void ReconnectGate(float fTimeElaps,unsigned int nTimerID );
	CTimerManager* GetTimerMgr(){ return m_pTimerMgr ;}
protected:
	bool SendMsgToGate(const char* pBuffer , unsigned int nLen);
	bool SendMsgToDB(const char* pBuffer , unsigned int nLen);
	void TryConnect(bool bGate );
protected:
	stServerInfo m_stGateServer ;

	stServerInfo m_stDBServer ;

	CNetWorkMgr* m_pNetWork;

	CLoginPeerMgr* m_pPeerMgr ;

	CTimer* m_pReconnctGate ;
	CTimer* m_pReconnectDB ;

	CTimerManager* m_pTimerMgr;

	CSeverConfigMgr m_stSvrConfigMgr ;
};