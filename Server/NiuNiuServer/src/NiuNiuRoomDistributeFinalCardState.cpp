#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuRoomGameResult.h"
void CNiuNiuRoomDistributeFinalCardState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	// distribute final card ;
	uint8_t nPlayerCnt = 0 ;
	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			pRoomPlayer->onGetCard(0,m_pRoom->getPoker()->GetCardWithCompositeNum()) ;
			++nPlayerCnt;
		}
	}

	// send msg ;
	setStateDuringTime(nPlayerCnt * TIME_DISTRIBUTE_ONE_PUBLIC_CARD );
}

void CNiuNiuRoomDistributeFinalCardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomGameResultState::eStateID) ;
}