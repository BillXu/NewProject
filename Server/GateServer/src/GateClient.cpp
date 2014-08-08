#include "GateClient.h"
#include "GateServer.h"
#define TIME_WAIT_FOR_RECONNECTE 60
stGateClient::stGateClient()
{
	nSessionId = 0 ;
	nNetWorkID = RakNet::UNASSIGNED_RAKNET_GUID ;
	pTimerForReconnect = NULL ;
}
void stGateClient::Reset(unsigned int nSessionID , RakNet::RakNetGUID& nNetWorkID )
{
	this->nSessionId = nSessionID ;
	this->nNetWorkID = nNetWorkID ;
	if ( pTimerForReconnect )
	{
		pTimerForReconnect->Stop();
	}
}

void stGateClient::TimeUpForReconnect(float fTimeElaps,unsigned int nTimerID)
{
	pTimerForReconnect->Stop() ;
	CGateServer::SharedGateServer()->GetClientMgr()->OnClientWaitReconnectTimeUp(this) ;
}

void stGateClient::StartWaitForReconnect()
{
	if ( pTimerForReconnect )
	{
		pTimerForReconnect->Reset();
		pTimerForReconnect->Start() ;
		return ;
	}

	pTimerForReconnect = CGateServer::SharedGateServer()->GetTimerMgr()->AddTimer(this,cc_selector_timer(stGateClient::TimeUpForReconnect));
	pTimerForReconnect->SetInterval(TIME_WAIT_FOR_RECONNECTE) ;
	pTimerForReconnect->Start() ;
}

void stGateClient::SetNewWorkID( RakNet::RakNetGUID& nNetWorkID)
{
	this->nNetWorkID = nNetWorkID ;
	if ( pTimerForReconnect )
	{
		pTimerForReconnect->Stop();
	}
}