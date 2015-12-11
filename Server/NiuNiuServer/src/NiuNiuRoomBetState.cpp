#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoom.h"
void CNiuNiuRoomBetState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;	
	setStateDuringTime(5) ;
}

void CNiuNiuRoomBetState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomDistributeFinalCardState::eStateID);
}

bool CNiuNiuRoomBetState::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}
	return false ;
}