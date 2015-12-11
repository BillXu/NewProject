#include "NiuNiuRoomGameResult.h"
#include "NiuNiuRoomWaitJoinState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuPeerCard.h"
void CNiuNiuRoomGameResultState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	// caculate result ;
	CNiuNiuRoomPlayer* pBanker = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(m_pRoom->getBankerIdx()) ;
	assert(pBanker && "why banker is null ?");

	// send result msg ;
	setStateDuringTime(8) ;
}

void CNiuNiuRoomGameResultState::onStateDuringTimeUp()
{
	m_pRoom->onGameDidEnd();
	m_pRoom->goToState(CNiuNiuRoomWaitJoinState::eStateID) ;
}