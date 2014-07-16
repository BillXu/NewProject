#pragma once
#include "ServerNetwork.h"
#include "ServerConfig.h"
#include "PushNotificationThread.h"
class CPushNotificationServer
	:public CServerNetworkDelegate
{
public:
	~CPushNotificationServer();
	bool Init();
	virtual bool OnMessage( RakNet::Packet* pData );
	virtual void OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData );
	virtual void OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData );
	void Update();
	void StopServer(){ m_bRunning = false ; }
protected:
	CPushNotificationThread m_nPushThread ;
	CServerNetwork* m_pSvrNetWork ;
	CSeverConfigMgr m_ServerConfigMgr ;
	bool m_bRunning ;
};