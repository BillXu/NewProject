#include "NiuNiuRoomTryBankerState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomRandBankerState.h"
void CNiuNiuRoomTryBanker::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	m_nBiggestTimeTryBanker = 0 ;
	setStateDuringTime(8) ;
}

bool CNiuNiuRoomTryBanker::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoomState::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	if ( prealMsg->usMsgType )
	{
		// recive msg ;
	}
}

void CNiuNiuRoomTryBanker::onStateDuringTimeUp()
{
	CNiuNiuRoom::LIST_SITDOWN_PLAYERS vMaybeBanker ;
	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pPlayer && pPlayer->isHaveState(eRoomPeer_CanAct) && pPlayer->getTryBankerTimes() == m_nBiggestTimeTryBanker )
		{
			vMaybeBanker.push_back(pPlayer) ;
		}
	}

	m_pRoom->setBetBottomTimes(m_nBiggestTimeTryBanker);
	if ( vMaybeBanker.size() == 1 )
	{
		m_pRoom->setBankerIdx(vMaybeBanker.front()->getIdx()) ;
		m_pRoom->goToState(CNiuNiuRoomBetState::eStateID);
		// send msg tell banker idx ;
	}
	else if ( vMaybeBanker.size() > 1 )
	{
		m_pRoom->goToState(CNiuNiuRoomRandBankerState::eStateID) ;
	}
}