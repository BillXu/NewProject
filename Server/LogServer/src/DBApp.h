#pragma once
#include "ServerNetwork.h"
#include "ServerConfig.h"
class CDBManager ;
class CDataBaseThread ;
class CDBServerApp
	:public CServerNetworkDelegate
{
public:
	CDBServerApp();
	~CDBServerApp();
	void Init();
	bool MainLoop();
	// net delegate
	virtual bool OnMessage( RakNet::Packet* pData );
	virtual void OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData );
	virtual void OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData );
	void SendMsg(const char* pBuffer, int nLen, RakNet::RakNetGUID& nTarget );
	bool IsRunning(){ return m_bRunning ;}
	void Stop(){ m_bRunning = false ;}
	void OnExit();
	CDataBaseThread* GetDBThread(){ return m_pDBWorkThread ; }
protected:
	CServerNetwork* m_pNetWork ;
	CDBManager* m_pDBManager ;
	CDataBaseThread* m_pDBWorkThread ;
	bool m_bRunning ;

	// server config 
	CSeverConfigMgr m_stSvrConfigMgr ;
};