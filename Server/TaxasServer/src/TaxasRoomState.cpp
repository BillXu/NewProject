#include "TaxasRoomState.h"
#include "TaxasRoom.h"
#include "LogManager.h"
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include "TaxasServerApp.h"
#include <time.h>
#include "TaxasPlayer.h"
// start game private card
void CTaxasStateStartGame::enterState(IRoom* pRoom )
{
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStatePrivateCard start game");
	m_pRoom = (CTaxasRoom*)pRoom ;
	float fTime = ( TIME_TAXAS_DISTRIBUTE_HOLD_CARD_DELAY * (m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * 2 - 1) ) + TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD;
	if ( fTime < 0 )
	{
		fTime = 0 ;
		CLogMgr::SharedLogMgr()->ErrorLog("distribute hold card time < 0 peer = %d",m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct));
	}
	fTime += 1.5 ;
	setStateDuringTime(fTime);
	m_pRoom->startGame();
}

void CTaxasStateStartGame::onStateDuringTimeUp()
{
	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("just begin game , why no player want coin room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver) ;
		return ;
	}

	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->goToState(eRoomState_TP_Beting) ;
	}
	else
	{
		m_pRoom->goToState(eRoomState_TP_OneRoundBetEndResult) ;
	}
}

// player bet state 
void CTaxasStatePlayerBet::enterState(IRoom* pRoom)
{
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d enter CTaxasStatePlayerBet",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
	m_bIsDoFinished = false ;
	m_pRoom->InformPlayerAct();
	resetStateTime();
}

void CTaxasStatePlayerBet::update(float fDelte )
{
	IRoomState::update(fDelte);
	if ( m_bIsDoFinished == false )
	{
		if ( m_pRoom->IsThisRoundBetOK() )
		{
			m_bIsDoFinished = true ;
			if ( m_pRoom->getMostBetCoinThisRound() > 0 )
			{
				setStateDuringTime(TIME_PLAYER_BET_COIN_ANI + 0.2f);
			}
			else
			{
				setStateDuringTime(0);
			}

			CLogMgr::SharedLogMgr()->PrintLog("some other player act , invoke bet state end room id = %d",m_pRoom->getRoomID()) ;
		}
	}
}

void CTaxasStatePlayerBet::onStateDuringTimeUp()
{
	if ( !m_bIsDoFinished )  // wait player act time out 
	{
		CLogMgr::SharedLogMgr()->PrintLog("wait time out");
		m_pRoom->OnPlayerActTimeOut() ;
		return ;
	}
 
	// bet state end ;
	if ( m_pRoom->getMostBetCoinThisRound() > 0 )
	{
		m_pRoom->goToState(eRoomState_TP_OneRoundBetEndResult);
		return ;
	}

	uint8_t nWaitCaPlyCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCaPlyCnt == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why all player leave in one minite room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver);
		return ;
	}

	if ( nWaitCaPlyCnt > 1 )  // go on distribute public card 
	{
		if ( m_pRoom->IsPublicDistributeFinish() ) // already distribute all card
		{
			CLogMgr::SharedLogMgr()->PrintLog(" distrubute pubic finsh") ;
			m_pRoom->goToState(eRoomState_TP_GameResult);
		}
		else
		{
			m_pRoom->goToState(eRoomState_TP_PublicCard );
		}
	}
	else  // only one player just go to cacuate coin ;
	{
		CLogMgr::SharedLogMgr()->PrintLog(" wait caculate only one  ") ;
		m_pRoom->goToState(eRoomState_TP_GameResult);
	}
}

void CTaxasStatePlayerBet::resetStateTime()
{
	setStateDuringTime(TIME_TAXAS_BET + 2);
}

bool CTaxasStatePlayerBet::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_PLAYER_LEAVE_ROOM:
	case MSG_PLAYER_STANDUP:
		{
			CLogMgr::SharedLogMgr()->PrintLog("some player standup");
			//warnning: must return false we can not capture this message ;
			auto pPlayer = (CTaxasPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
			if ( pPlayer == nullptr )
			{
				return false ;
			}

			uint8_t nSeatIdx = pPlayer->getIdx();
			if ( nSeatIdx >= m_pRoom->getSeatCount() )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why have invalid seat idx = %d",nSeatIdx);
				return false ;
			}

			if ( nSeatIdx == m_pRoom->GetCurWaitActPlayerIdx() )
			{
				CLogMgr::SharedLogMgr()->SystemLog("client should send give up msg , before player standup or leave room  player uid = %d",pPlayer->getUserUID()) ;
				// do something ;
				uint32_t nValue = 0 ;
				m_pRoom->OnPlayerAction(pPlayer->getIdx(),eRoomPeerAction_GiveUp,nValue);
				playerDoActOver() ;
				return false ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->PrintLog("some player standup idx = %d",nSeatIdx);
				return false ;
			}
		}
		break;
	case MSG_TP_PLAYER_ACT:
		{
			stMsgTaxasPlayerActRet msgBack ;
			auto pPlayer = (CTaxasPlayer*)m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);
			if ( pPlayer == nullptr )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("cur act player not sit down room id = %d",m_pRoom->getRoomID()) ;
				msgBack.nRet = 2 ;
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				break; 
			}

			uint8_t nSeatIdx = pPlayer->getIdx();
			if ( nSeatIdx >= m_pRoom->getSeatCount() )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("MSG_TP_PLAYER_ACT why have invalid seat idx = %d",nSeatIdx);
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				break; 
			}

			
			stMsgTaxasPlayerAct* pAct = (stMsgTaxasPlayerAct*)prealMsg ;
			msgBack.nRet = m_pRoom->OnPlayerAction( nSeatIdx,(eRoomPeerAction)pAct->nPlayerAct,pAct->nValue) ;
			if ( msgBack.nRet == 0 )
			{
				playerDoActOver();
				CLogMgr::SharedLogMgr()->PrintLog("room id = %d player idx = %d do act = %d, value = %u",m_pRoom->getRoomID(), nSeatIdx,pAct->nPlayerAct,pAct->nValue ) ;
			}
			else
			{
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				CLogMgr::SharedLogMgr()->ErrorLog("player idx = %d act error ret = %d , room id = %d",nSeatIdx,msgBack.nRet,m_pRoom->getRoomID()); 
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CTaxasStatePlayerBet::playerDoActOver()
{
	if ( m_pRoom->IsThisRoundBetOK() == false )
	{
		m_pRoom->InformPlayerAct() ;
		resetStateTime() ;
	}
	else
	{
		m_bIsDoFinished = true ;
		if ( m_pRoom->getMostBetCoinThisRound() > 0 )
		{
			setStateDuringTime(TIME_PLAYER_BET_COIN_ANI + 0.2f);
		}
		else
		{
			setStateDuringTime(0);
		}
		CLogMgr::SharedLogMgr()->PrintLog("current player act , invoke bet state end room id = %d",m_pRoom->getRoomID()) ;
	}
}

// caculate vice pool
void CTaxasStateOneRoundBetEndResult::enterState(IRoom* pRoom)
{
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d enter CTaxasStateOneRoundBetEndResult",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
	uint8_t nCnt = m_pRoom->CaculateOneRoundPool() ;
	float fTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL + TIME_TAXAS_MAKE_VICE_POOLS * nCnt + 1.2f ; // if produced vice pool , need more time ;
	setStateDuringTime(fTime) ;
}

void CTaxasStateOneRoundBetEndResult::onStateDuringTimeUp()
{
	uint8_t nWaitCaPlyCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCaPlyCnt == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why all player leave in one minite room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver);
		return ;
	}

	if ( m_pRoom->IsPublicDistributeFinish() )
	{
		m_pRoom->goToState(eRoomState_TP_GameResult) ;
	}
	else
	{
		if ( nWaitCaPlyCnt >= 2 )
		{
			m_pRoom->goToState(eRoomState_TP_PublicCard) ;
		}
		else
		{
			m_pRoom->goToState(eRoomState_TP_GameResult) ;
		}
	}
}

// public card
void CTaxasStatePublicCard::enterState(IRoom* pRoom)
{
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d enter CTaxasStatePublicCard",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
	float fTime = TIME_DISTRIBUTE_ONE_PUBLIC_CARD * m_pRoom->DistributePublicCard() + 1.0f;
	setStateDuringTime(fTime) ;
}

void CTaxasStatePublicCard::onStateDuringTimeUp()
{
	if ( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->PreparePlayersForThisRoundBet();
		m_pRoom->goToState(eRoomState_TP_Beting) ;
		return ;
	}

	uint8_t nWaitCaPlyCnt = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCaPlyCnt == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why all player leave in one minite room id = %d",m_pRoom->getRoomID()) ;
		m_pRoom->goToState(eRoomState_DidGameOver);
		return ;
	}
 
	if ( nWaitCaPlyCnt >= 2 && (m_pRoom->IsPublicDistributeFinish() == false ) )
	{
		m_pRoom->goToState(eRoomState_TP_PublicCard) ;
	}
	else
	{
		m_pRoom->goToState(eRoomState_TP_GameResult) ;
	}
}

// game result 
void CTaxasStateGameResult::enterState(IRoom* pRoom)
{
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d enter CTaxasStateGameResult",pRoom->getRoomID() );
	m_pRoom = (CTaxasRoom*)pRoom ;
    uint8_t nWaitCal = (uint8_t)m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
	float fTime = 0 ;
	if ( nWaitCal > 1 )
	{
		fTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + TIME_TAXAS_SHOW_BEST_CARD + 1;
	}
	else
	{
		fTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + 1.3f  + TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL ;  // all players give up ;
	}
	setStateDuringTime(fTime) ;
}

void CTaxasStateGameResult::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_DidGameOver) ;
}