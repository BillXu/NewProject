#pragma once
#include "RakNetTypes.h"
#include "RakPeerInterface.h"
#include <list>
class CServerNetworkDelegate
{
public:
	CServerNetworkDelegate():m_nPriority(0){}
	virtual ~CServerNetworkDelegate(){}
	virtual bool OnMessage( RakNet::Packet* pData ) = 0;
	virtual void OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData ){}
	virtual void OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData ){}
	void SetPriority(unsigned int nPriority );
	unsigned int GetPriority(){ return m_nPriority ; }
protected:
	unsigned int m_nPriority ;
};

class CServerNetwork
{
public:
	typedef std::list<CServerNetworkDelegate*> LIST_DELEGATE ;
	typedef bool (CServerNetwork::*lpFunc)(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData);
public:
	static CServerNetwork* SharedNetwork();
	CServerNetwork();
	~CServerNetwork();
	bool StartupNetwork( unsigned short nPort , int nMaxInComming, const char* pIncomingPassword = NULL );
	void ShutDown();
	void RecieveMsg();
	void SendMsg(const char* pData , int nLength , RakNet::RakNetGUID& nSendToOrExcpet ,bool bBroadcast );
	void ClosePeerConnection(RakNet::RakNetGUID& nPeerToClose);
	void AddDelegate(CServerNetworkDelegate* pDelegate , unsigned int nPriority = 0 );
	void RemoveDelegate(CServerNetworkDelegate* pDelegate );
protected:
	bool OnNewPeerConnected(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData );
	bool OnPeerDisconnected(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData );
	bool OnLogicMessage(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData);
	void EnumDelegate( lpFunc pFunc, RakNet::Packet* pData );
protected:
	RakNet::RakPeerInterface* m_pNetPeer ;
	LIST_DELEGATE m_vAllDelegates ;
};