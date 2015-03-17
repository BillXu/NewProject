#pragma once
#include "NetWorkManager.h"
#include "ServerConfig.h"
class CDBManager ;
class CDataBaseThread ;
class CDBServerApp
	:public CNetMessageDelegate
{
public:
	CDBServerApp();
	~CDBServerApp();
	void Init();
	bool MainLoop();
	// net delegate
	virtual bool OnMessage( Packet* pMsg );
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	void SendMsg(const char* pBuffer, int nLen,uint32_t nSessionID = 0 );
	bool IsRunning(){ return m_bRunning ;}
	void Stop(){ m_bRunning = false ;}
	void OnExit();
	CDataBaseThread* GetDBThread(){ return m_pDBWorkThread ; }
protected:
	CNetWorkMgr* m_pNetWork ;
	CDBManager* m_pDBManager ;
	CDataBaseThread* m_pDBWorkThread ;
	bool m_bRunning ;
	CONNECT_ID m_nCenterSvrConnectID ;

	// server config 
	CSeverConfigMgr m_stSvrConfigMgr ;

	char m_pSendBuffer[MAX_MSG_BUFFER_LEN] ;
};