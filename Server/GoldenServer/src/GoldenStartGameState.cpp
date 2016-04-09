#include "GoldenStartGameState.h"
#include "AutoBuffer.h"
#include "LogManager.h"
#include "GoldenMessageDefine.h"
#include "GoldenRoomPlayer.h"
#include "GoldenRoom.h"
void CGoldenStartGameState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;

	// distribute card ;
	uint8_t nPlayerCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ;
	// send msg ;
	stMsgGoldenDistribute msgForCard ;
	msgForCard.nCnt = nPlayerCnt;
	CAutoBuffer buffer(sizeof(msgForCard) + msgForCard.nCnt * sizeof(stGoldenHoldPeerCard)) ;
	buffer.addContent(&msgForCard,sizeof(msgForCard)) ;
	for ( uint8_t nIdx = 0 ; nIdx < m_pRoom->getSeatCount() ; ++nIdx )
	{
		CGoldenRoomPlayer* pRoomPlayer = (CGoldenRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			stGoldenHoldPeerCard item ;
			item.nIdx = pRoomPlayer->getIdx();
			for ( uint8_t nIdx = 0 ; nIdx < GOLDEN_PEER_CARD ; ++nIdx )
			{
				item.vCard[nIdx] = pRoomPlayer->getCardByIdx(nIdx);
			}
			buffer.addContent(&item,sizeof(item)) ;
		}
	}

	m_pRoom->sendRoomMsg((stMsg*)buffer.getBufferPtr(),buffer.getContentSize()) ;

	setStateDuringTime( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * TIME_GOLDEN_DISTRIBUTE_CARD_PER_PLAYER );
	CLogMgr::SharedLogMgr()->PrintLog("player cnt = %d room id = %d , distribute card",nPlayerCnt,m_pRoom->getRoomID());
}

void CGoldenStartGameState::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_Golden_Bet ) ;
	((CGoldenRoom*)m_pRoom)->informPlayerAct(false);
}