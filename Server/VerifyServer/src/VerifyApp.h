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
	virtual bool OnMessage( RakNet::Packet* pData );
	virtual void OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData );
	virtual void OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData );
protected:
	LIST_VERIFY_REQUEST m_vListRequest ;
	CServerNetwork* m_pNetwork ;
	CAppleVerifyManager m_AppleVerifyMgr ;
	CDBVerifyManager m_DBVerifyMgr ;

	CSeverConfigMgr m_stSvrConfigMgr ;
};