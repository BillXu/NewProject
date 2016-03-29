#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuRoomPlayerCaculateCardState.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "LogManager.h"
void CNiuNiuRoomDistributeFinalCardState::enterState(IRoom* pRoom)
{
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d start final card ",pRoom->getRoomID()) ;
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	// distribute final card ;
	uint8_t nPlayerCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ; ;
	uint8_t nSeatCnt = (uint8_t)m_pRoom->getSeatCount() ;
	// send msg ;
	stMsgNNDistributeFinalCard msgFinalCard ;
	msgFinalCard.nPlayerCnt = nPlayerCnt ;
	CAutoBuffer auBuffer(sizeof(msgFinalCard) + msgFinalCard.nPlayerCnt * sizeof(stDistributeFinalCardItem)) ;
	auBuffer.addContent(&msgFinalCard,sizeof(msgFinalCard)) ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			stDistributeFinalCardItem item ;
			item.nCardCompsitNum = pRoomPlayer->getCardByIdx(4) ;
			item.nPlayerIdx = pRoomPlayer->getIdx() ;
			auBuffer.addContent(&item,sizeof(item)) ;
			CLogMgr::SharedLogMgr()->PrintLog("final card for idx = %d , card = %d",item.nPlayerIdx,item.nCardCompsitNum) ;
		}
	}
	m_pRoom->sendRoomMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;

	setStateDuringTime(nPlayerCnt * TIME_NIUNIU_DISTRIBUTE_FINAL_CARD_PER_PLAYER );
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d distribute final card, player cnt = %d",m_pRoom->getRoomID(),nPlayerCnt) ;
}

void CNiuNiuRoomDistributeFinalCardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomStatePlayerCaculateCardState::eStateID) ;
}