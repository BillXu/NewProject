#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "LogManager.h"
void CNiuNiuRoomBetState::enterState(IRoom* pRoom)
{
	m_pRoom = (CNiuNiuRoom*)pRoom ;	
	setStateDuringTime(TIME_NIUNIU_PLAYER_BET) ;
	m_nLeftBetPlayerCnt = m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) ;
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d start bet ",m_pRoom->getRoomID()) ;
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

	if ( prealMsg->usMsgType == MSG_NN_PLAYER_BET )
	{
		stMsgNNPlayerBet* pBet = (stMsgNNPlayerBet*)prealMsg ;
		stMsgNNPlayerBetRet msgBack ;
		uint64_t nBetCoin = m_pRoom->getBaseBet() * m_pRoom->getBetBottomTimes() * pBet->nBetTimes ;
		CNiuNiuRoomPlayer* pPlayer = (CNiuNiuRoomPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
		if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			msgBack.nRet = 3 ;
		}
		else if ( (uint64_t)pPlayer->getCoin() < nBetCoin )
		{
			msgBack.nRet = 2 ;
		}
		else if ( m_pRoom->getBankCoinLimitForBet() < nBetCoin * m_pRoom->getMaxRate() )
		{
			msgBack.nRet = 1 ;
		}
		else 
		{
			msgBack.nRet = 0 ;
			pPlayer->setBetTimes(pBet->nBetTimes) ;
			stMsgNNBet msgRoomBet ;
			msgRoomBet.nBetTimes = pBet->nBetTimes ;
			msgRoomBet.nPlayerIdx = pPlayer->getIdx() ;
			m_pRoom->sendRoomMsg(&msgRoomBet,sizeof(msgRoomBet)) ;
			m_pRoom->setBankCoinLimitForBet(m_pRoom->getBankCoinLimitForBet() - nBetCoin ) ;
		}
		m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
		if ( --m_nLeftBetPlayerCnt == 0 )  // end bet state ;
		{
			onStateDuringTimeUp() ;
		}
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d bet times = %d, ret = %d",pPlayer->getUserUID(),pBet->nBetTimes,msgBack.nRet) ;
		return true ;
	}
	return false ;
}