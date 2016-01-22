#include "NiuNiuRoomGameResult.h"
#include "NiuNiuRoomWaitJoinState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuPeerCard.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "LogManager.h"
void CNiuNiuRoomGameResultState::enterState(IRoom* pRoom)
{
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d game result ",pRoom->getRoomID()) ;
	m_pRoom = (CNiuNiuRoom*)pRoom ;
	// caculate result ;
	CNiuNiuRoomPlayer* pBanker = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(m_pRoom->getBankerIdx()) ;
	assert(pBanker && "why banker is null ?");
	CLogMgr::SharedLogMgr()->PrintLog("banker coin = %I64d",pBanker->getCoin()) ;
	// send result msg ;
	stMsgNNGameResult msgResult ;
	msgResult.nPlayerCnt = m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ;

	CAutoBuffer auBuffer(sizeof(msgResult) + msgResult.nPlayerCnt * sizeof(stNNGameResultItem));
	auBuffer.addContent(&msgResult,sizeof(msgResult)) ;

	uint8_t nSeatCnt = m_pRoom->getSeatCount() ;
	int64_t nBankerOffset = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt  ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getPlayerByIdx(nIdx) ;
		if ( pPlayer == nullptr || pPlayer == pBanker || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			continue; 
		}
		
		if ( pPlayer->getBetTimes() == 0 )
		{
			pPlayer->setBetTimes(5) ;
		}

		uint64_t nBaseBetCoin = pPlayer->getBetTimes() * m_pRoom->getBaseBet() * m_pRoom->getBetBottomTimes() ;
		int64_t nPlayerOffsetCoin = 0 ;
		if ( pPlayer->getPeerCard()->pk(pBanker->getPeerCard()) == IPeerCard::PK_RESULT_WIN )
		{
			nPlayerOffsetCoin = nBaseBetCoin * m_pRoom->getReateByNiNiuType(pPlayer->getPeerCard()->getType(),pPlayer->getPeerCard()->getPoint()) ;
		}
		else
		{
			nPlayerOffsetCoin = -1 * nBaseBetCoin * m_pRoom->getReateByNiNiuType(pBanker->getPeerCard()->getType(),pBanker->getPeerCard()->getPoint()) ;
		}

		nBankerOffset -= nPlayerOffsetCoin ;
		pBanker->setCoin(pBanker->getCoin() - nPlayerOffsetCoin ) ;
		pPlayer->setCoin(pPlayer->getCoin() + nPlayerOffsetCoin ) ;

		stNNGameResultItem item ;
		item.nFinalCoin = pPlayer->getCoin() ;
		item.nOffsetCoin = nPlayerOffsetCoin ;
		item.nPlayerIdx = pPlayer->getIdx() ;
		auBuffer.addContent(&item,sizeof(item)) ;
		pPlayer->setCoinOffsetThisGame(nPlayerOffsetCoin);
		CLogMgr::SharedLogMgr()->PrintLog("result player idx = %d , finalCoin = %llu, offset coin = %I64d",item.nPlayerIdx,item.nFinalCoin,item.nOffsetCoin) ;
	}

	stNNGameResultItem item ;
	item.nFinalCoin = pBanker->getCoin() ;
	item.nOffsetCoin = nBankerOffset ;
	item.nPlayerIdx = pBanker->getIdx() ;
	auBuffer.addContent(&item,sizeof(item)) ;
	pBanker->setCoinOffsetThisGame(nBankerOffset);
	CLogMgr::SharedLogMgr()->PrintLog("result player idx = %d , finalCoin = %llu, offset coin = %I64d",item.nPlayerIdx,item.nFinalCoin,item.nOffsetCoin) ;

	m_pRoom->sendRoomMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;

	setStateDuringTime(TIME_NIUNIU_GAME_RESULT_PER_PLAYER * msgResult.nPlayerCnt + TIME_NIUNIU_GAME_RESULT_EXT ) ;
}

void CNiuNiuRoomGameResultState::onStateDuringTimeUp()
{
	m_pRoom->onGameDidEnd();
	m_pRoom->goToState(CNiuNiuRoomWaitJoinState::eStateID) ;
}