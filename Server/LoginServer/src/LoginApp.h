#pragma once 
#include "NetWorkManager.h"
#include "Timer.h"
#include "ServerConfig.h"
class CDBManager ;
class CDataBaseThread ;
class CLoginApp
	:public CNetMessageDelegate
{
public:
	CLoginApp();
	~CLoginApp();
	void Init();
	void MainLoop();
	virtual bool OnMessage( Packet* pMsg );
	virtual bool OnLostSever(Packet* pMsg);
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg);
	bool SendMsg( const char* pBuffer , unsigned int nLen, uint32_t nSessioniD );
protected:
	CONNECT_ID m_nCenterSvrNetworkID;
	CDataBaseThread* m_pDBThread;
	CDBManager* m_pDBMgr ;
	CSeverConfigMgr m_stSvrConfigMgr ;
	CNetWorkMgr* m_pNetWork;
	char m_pSendBuffer[MAX_MSG_BUFFER_LEN] ;
};