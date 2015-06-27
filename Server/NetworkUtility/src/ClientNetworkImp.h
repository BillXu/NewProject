#pragma once
#include "INetwork.h"
class CClientNetwork
	:public INetwork
{
public:
	struct stConnectMission
	{
		char cIP[20];
		unsigned short nPort ;
		CONNECT_ID nConnectID ;
	};
	typedef std::vector<stConnectMission*> VEC_CONNECT_MISSION ; 
public:
	void Start();
	CONNECT_ID ConnectToServer(const char* pIP, unsigned short nPort );
protected:
	void DoConnectMission();
	void Run();
protected:
	zsummer::thread4z::CLock m_lockConnectMission; 
	VEC_CONNECT_MISSION	m_vWillConnect ;
};