#pragma once
#include "header.h"
class CSession;
class INetwork
	:public IIOServerCallback,public CThread
{
public:
	typedef std::map<CONNECT_ID,CSession*> MAP_ID_SESSION ;
	typedef std::vector<CSession*> VEC_SESSION;
	typedef std::vector<Packet*> VEC_PACKET;
public:
	INetwork();
	virtual ~INetwork();
	bool GetAllPacket(VEC_PACKET& vOutPackets ); // must delete out side ;
	bool SendMsg(unsigned char* pBuffer , unsigned short nLen ,CONNECT_ID nSendToOrExcpt , bool bBorcast = false) ;
	void CloseConnection(CONNECT_ID nConnectID );
	char* GetIPStringByConnectID( CONNECT_ID nConnectID );
	void Stop();
	void Start();
	bool OnPost(void *pUser){return true ;}
	void CheckHeatBeat();
protected:
	void Run();
	void OnNewSession(CSession* pAddClient,CONNECT_ID nConnectID ) ;
	void RemoveSession(CONNECT_ID nConnectID );
	void AddPacket(Packet* pPacket ) ;
	void ProcessSendMsg();
protected:
	friend class CSession ;
protected:
	IIOServer * m_ios;
	//! 
	bool m_bRunning;

	MAP_ID_SESSION m_vAllSessions ;
	zsummer::thread4z::CLock m_lockAllSession;

	VEC_PACKET m_vRecievedPackets ;
	zsummer::thread4z::CLock m_lockRecvPackets;

	VEC_PACKET m_vWillSendPackets;
	zsummer::thread4z::CLock m_lockSendPackets;
};