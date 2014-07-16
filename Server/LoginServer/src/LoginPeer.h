#pragma once
#include "RakNetTypes.h"
class CLoginPeerMgr ;
class CLoginPeer
{
public:
	CLoginPeer(CLoginPeerMgr* pPeerMgr );
	~CLoginPeer();
	void OnMessage(RakNet::Packet* packet );
	void Reset(unsigned int nSessionID );
	void SendMsgToDB(const char* pBuffer , unsigned int nLen );
	void SendMsgToGate(const char* pBuffer , unsigned int nLen );
protected:
	unsigned int m_nSessionID ;
	CLoginPeerMgr* m_pPeerMgr ;
};