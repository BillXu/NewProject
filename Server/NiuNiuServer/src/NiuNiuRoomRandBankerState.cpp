#include "NiuNiuRoomRandBankerState.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
void CNiuNiuRoomRandBankerState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;

	CNiuNiuRoom::LIST_SITDOWN_PLAYERS vMaybeBanker ;
	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pPlayer && pPlayer->isHaveState(eRoomPeer_CanAct) && pPlayer->getTryBankerTimes() == m_pRoom->getBetBottomTimes() )
		{
			vMaybeBanker.push_back(pPlayer) ;
		}
	}

	uint8_t nBankerCntIdx = rand() % vMaybeBanker.size();
	CNiuNiuRoom::LIST_SITDOWN_PLAYERS::iterator iter = vMaybeBanker.begin() ;
	for ( uint8_t nMayBankerIdx = 0 ; iter != vMaybeBanker.end() ; ++iter, ++nMayBankerIdx )
	{
		if ( nBankerCntIdx == nMayBankerIdx )
		{
			m_pRoom->setBankerIdx((*iter)->getIdx()) ;
			break; 
		}
	}

	setStateDuringTime( 0.5 * vMaybeBanker.size() );
}

void CNiuNiuRoomRandBankerState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomBetState::eStateID);
}