#pragma once
#include "ServerNetwork.h"
#include "Timer.h"
struct stGateClient
	:public CTimerDelegate
{
public:
	stGateClient();
	void Reset( unsigned int nSessionID , RakNet::RakNetGUID& nNetWorkID );
	void TimeUpForReconnect( float fTimeElaps,unsigned int nTimerID);
	void StartWaitForReconnect();
	void SetNewWorkID( RakNet::RakNetGUID& nNetWorkID);
public:
	unsigned int nSessionId ;
	RakNet::RakNetGUID nNetWorkID ;
	CTimer* pTimerForReconnect ;
};