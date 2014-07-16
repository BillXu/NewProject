#pragma once
#include <map>
#include <list>
#include "ServerNetwork.h"
#include "CommonDefine.h"
struct stGateClient ;
class CGateClientMgr
	:public CServerNetworkDelegate
{
public:
	typedef std::map<unsigned int ,stGateClient*> MAP_SESSIONID_GATE_CLIENT ;
	typedef std::map<RakNet::RakNetGUID, stGateClient*> MAP_NETWORKID_GATE_CLIENT ;
	typedef std::list<stGateClient*> LIST_GATE_CLIENT ;
public:
	CGateClientMgr();
	~CGateClientMgr();
	virtual bool OnMessage( RakNet::Packet* pData ) ;
	virtual void OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData );
	virtual void OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData );
	void OnClientWaitReconnectTimeUp(stGateClient* pClient );
protected:
	void AddClientGate(stGateClient* pGateClient );
	void RemoveClientGate(stGateClient* pGateClient );
	stGateClient* GetReserverGateClient();
	stGateClient* GetGateClientBySessionID(unsigned int nSessionID);
	stGateClient* GetGateClientByNetWorkID(RakNet::RakNetGUID& nNetWorkID );
	bool CheckServerStateOk(stGateClient* pClient, eServerType eCheckType ) ;
protected:
	friend struct stGateClient ;
	static unsigned int s_nSeesionIDProducer ;
protected:
	MAP_NETWORKID_GATE_CLIENT m_vNetWorkIDGateClient ;
	MAP_SESSIONID_GATE_CLIENT m_vSessionGateClient ;
	LIST_GATE_CLIENT m_vGateClientReserver ;
	RakNet::RakNetGUID m_nGameServerNetWorkID ;
	RakNet::RakNetGUID m_nLoginServerNetWorkID ;

	char m_pMsgBuffer[MAX_MSG_BUFFER_LEN] ;
};