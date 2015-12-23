#include "NiuNiuRoomDistribute4CardState.h"
#include "NiuNiuRoomTryBankerState.h"
#include "ISitableRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "LogManager.h"
void CNiuNiuRoomDistribute4CardState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;
	m_pRoom->onGameWillBegin();

	// distribute card ;
	uint8_t nPlayerCnt = 0 ;
	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = NIUNIU_HOLD_CARD_COUNT - 1 ;
			++nPlayerCnt;
			while ( nCardCount-- )
			{
				pRoomPlayer->onGetCard(nCardCount,m_pRoom->getPoker()->GetCardWithCompositeNum()) ;
			}
		}
	}

	// send msg ;
	stMsgNNDistriute4Card msgForCard ;
	msgForCard.nPlayerCnt = nPlayerCnt;
	CAutoBuffer buffer(sizeof(msgForCard) + msgForCard.nPlayerCnt * sizeof(stDistriuet4CardItem)) ;
	buffer.addContent(&msgForCard,sizeof(msgForCard)) ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			 stDistriuet4CardItem item ;
			 item.nSeatIdx = pRoomPlayer->getIdx();
			 for ( uint8_t nIdx = 0 ; nIdx < 4 ; ++nIdx )
			 {
				 item.vCardCompsitNum[nIdx] = pRoomPlayer->getCardByIdx(nIdx);
			 }
			 buffer.addContent(&item,sizeof(item)) ;
		}
	}

	m_pRoom->sendRoomMsg((stMsg*)buffer.getBufferPtr(),buffer.getContentSize()) ;

	setStateDuringTime( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * TIME_NIUNIU_DISTRIBUTE_4_CARD_PER_PLAYER );
	CLogMgr::SharedLogMgr()->PrintLog("player cnt = %d room id = %d , distribute 4 card",nPlayerCnt,m_pRoom->getRoomID());
}

void CNiuNiuRoomDistribute4CardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomTryBanker::eStateID) ;
}