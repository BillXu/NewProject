#include "NiuNiuRoomDistribute4CardState.h"
#include "NiuNiuRoomTryBankerState.h"
#include "ISitableRoom.h"
#include "NiuNiuRoomPlayer.h"
void CNiuNiuRoomDistribute4CardState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;
	m_pRoom->onGameWillBegin();

	// distribute card ;
	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = NIUNIU_HOLD_CARD_COUNT - 1 ;
			while ( nCardCount-- )
			{
				pRoomPlayer->onGetCard(nCardCount,m_pRoom->getPoker()->GetCardWithCompositeNum()) ;
			}
		}
	}

	// send msg ;

	setStateDuringTime( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * (NIUNIU_HOLD_CARD_COUNT - 1) * TIME_DISTRIBUTE_ONE_PUBLIC_CARD );
}

void CNiuNiuRoomDistribute4CardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomTryBanker::eStateID) ;
}