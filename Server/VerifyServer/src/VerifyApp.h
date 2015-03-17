#pragma once
#include "VerifyRequest.h"
#include "AppleVerifyManager.h"
#include "DBVerifyManager.h"
#include "ServerNetwork.h"
#include "ServerConfig.h"
class CVerifyApp
	:public CServerNetworkDelegate
{
public:
	CVerifyApp();
	~CVerifyApp();
	void MainLoop();
	void Init();
	stVerifyRequest* GetRequestToUse();
	void PushVerifyRequestToReuse(stVerifyRequest* pRequest );
	void FinishVerifyRequest(stVerifyRequest* pRequest);

	//---
	virtual bool OnMessage( Packet* pData );
	virtual void OnNewPeerConnected(CONNECT_ID nNewPeer, ConnectInfo* IpInfo );
	virtual void OnPeerDisconnected(CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo );
protected:
	LIST_VERIFY_REQUEST m_vListRequest ;
	CServerNetwork* m_pNetwork ;
	CAppleVerifyManager m_AppleVerifyMgr ;
	CDBVerifyManager m_DBVerifyMgr ;

	CSeverConfigMgr m_stSvrConfigMgr ;
};