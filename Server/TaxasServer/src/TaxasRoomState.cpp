#include "TaxasRoomState.h"
#include "TaxasRoom.h"
#include "LogManager.h"
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include "TaxasServerApp.h"
#include <time.h>
#include "TaxasPlayer.h"
//CTaxasBaseRoomState::CTaxasBaseRoomState()
//{
//	m_fDuringTime = 0 ;
//	m_pRoom = NULL ;
//}
//
//CTaxasBaseRoomState::~CTaxasBaseRoomState()
//{
//
//}
//
//bool CTaxasBaseRoomState::OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
//{
//	switch( prealMsg->usMsgType )
//	{
//	case MSG_TP_PLAYER_SIT_DOWN:
//		{
//			stMsgTaxasPlayerSitDown* pRet = (stMsgTaxasPlayerSitDown*)prealMsg ;
//			stMsgTaxasPlayerSitDownRet msgBack ;
//			msgBack.nRet = 0 ;
//			msgBack.nSeatIdx = pRet->nSeatIdx ;
//			if ( m_pRoom->IsPlayerInRoomWithSessionID(nPlayerSessionID) == false )
//			{
//				msgBack.nRet = 2;
//				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
//				CLogMgr::SharedLogMgr()->ErrorLog("you are not in room session id = %d",nPlayerSessionID ) ;
//				return true ;
//			}
//
//			if ( m_pRoom->isPlayerAlreadySitDown(nPlayerSessionID) )
//			{
//				msgBack.nRet = 4;
//				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
//				CLogMgr::SharedLogMgr()->ErrorLog("you are already sit down session id = %d",nPlayerSessionID ) ;
//				return true ;
//			}
//
//			if ( pRet->nSeatIdx >= m_pRoom->m_stRoomConfig.nMaxSeat || m_pRoom->m_vSitDownPlayers[pRet->nSeatIdx].IsInvalid() == false )
//			{
//				msgBack.nRet = 1;
//				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
//				CLogMgr::SharedLogMgr()->PrintLog("invalid seat idx = %d , can not sit down session id = %d",pRet->nSeatIdx,nPlayerSessionID ) ;
//				return true ;
//			}
//
//			if ( pRet->nTakeInMoney > m_pRoom->m_stRoomConfig.nMaxTakeInCoin || pRet->nTakeInMoney < m_pRoom->m_stRoomConfig.nMiniTakeInCoin )
//			{
//				msgBack.nRet = 3;
//				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
//				CLogMgr::SharedLogMgr()->PrintLog("invalid take in coin = %I64d , can not sit down session id = %d",pRet->nTakeInMoney,nPlayerSessionID ) ;
//				return true ;
//			}
//
//			stTaxasInRoomPeerDataExten* pData = m_pRoom->GetInRoomPlayerDataBySessionID(nPlayerSessionID);
//			if ( pData == nullptr )
//			{
//				msgBack.nRet = 2;
//				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
//				CLogMgr::SharedLogMgr()->ErrorLog("you are not in room session id = %d data is null",nPlayerSessionID ) ;
//				return true ;
//			}
//
//			m_pRoom->OnPlayerSitDown(pRet->nSeatIdx,nPlayerSessionID,pRet->nTakeInMoney) ;
//			CLogMgr::SharedLogMgr()->PrintLog("player seat idx = %d sit down want takein coin = %I64d",pRet->nSeatIdx,pRet->nTakeInMoney) ;
//			return true ;
//		}
//		break;
//	case MSG_TP_PLAYER_STAND_UP:
//		{
//			uint8_t nSeatIdx = m_pRoom->GetSeatIdxBySessionID(nPlayerSessionID);
//			if ( nSeatIdx >= m_pRoom->m_stRoomConfig.nMaxSeat )
//			{
//				CLogMgr::SharedLogMgr()->ErrorLog("you already stand up session id = %d",nPlayerSessionID ) ;
//				break;
//			}
//			m_pRoom->OnPlayerStandUp(nSeatIdx) ;
//			CLogMgr::SharedLogMgr()->PrintLog("player seat idx = %d stand up ",nSeatIdx );
//		}
//		break;
//	case MSG_TP_ORDER_LEAVE:
//	case MSG_TP_PLAYER_LEAVE:
//		{
//			if ( m_pRoom->IsPlayerInRoomWithSessionID(nPlayerSessionID) == false )
//			{
//				// already check by top level code ;
//				CLogMgr::SharedLogMgr()->ErrorLog("you are not in room so how you leave , session id = %d",nPlayerSessionID ) ;
//				break;
//			}
//			m_pRoom->OnPlayerLeaveRoom(nPlayerSessionID) ;
//			CLogMgr::SharedLogMgr()->PrintLog("player session id = %d leave room ",nPlayerSessionID ) ;
//		}
//		break;
//	default:
//		return false ;
//	}
//	return true ;
//}
//
//void CTaxasBaseRoomState::EnterState(CTaxasRoom* pRoom )
//{
//	m_pRoom = pRoom ;
//}
//
//void CTaxasBaseRoomState::LeaveState()
//{
//	m_fDuringTime = 0 ;
//}
//
//void CTaxasBaseRoomState::Update(float fDelte )
//{
//	m_fDuringTime -= fDelte ;
//	if ( m_fDuringTime <= 0 )
//	{
//		OnStateTimeOut();
//	}
//}
//
//void CTaxasStateDead::EnterState(CTaxasRoom* pRoom )
//{
//	CTaxasBaseRoomState::EnterState(pRoom);
//	// send inform ;
//	if ( pRoom->getOwnerUID() == MATCH_MGR_UID )  // match room 
//	{
//		pRoom->onMatchFinish();
//		m_MatchRoomDuringTime = pRoom->getDeadTime() - pRoom->getCreateTime();
//		m_fMatchRestarTime = TIME_MATCH_PAUSE ; // 30 min later start new match ;
//	}
//	else
//	{
//		pRoom->sendExpireInform();
//	}
//}
//
//void CTaxasStateDead::Update(float fDelte )
//{
//	if ( m_pRoom->isDeleteRoom() == false && m_pRoom->getOwnerUID() != MATCH_MGR_UID )
//	{
//		time_t deadTime = m_pRoom->getDeadTime() ;
//		if ( time(nullptr) - deadTime > 8640*3 ) // 3 days ;
//		{
//			m_pRoom->deleteRoom();
//		}
//	}
//
//	if ( m_pRoom->isRoomAlive() )
//	{
//		m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
//		return ;
//	}
//	else
//	{
//		if ( m_pRoom->getOwnerUID() == MATCH_MGR_UID )
//		{
//			m_fMatchRestarTime -= fDelte;
//			if ( m_fMatchRestarTime <= 0 )
//			{
//				m_pRoom->setCreateTime(time(nullptr));
//				m_pRoom->setDeadTime(m_pRoom->getCreateTime() + m_MatchRoomDuringTime) ;
//				m_pRoom->onMatchRestart();
//			}
//		}
//	}
//}
//
//// wait join state 
//void CTaxasStateWaitJoin::EnterState(CTaxasRoom* pRoom )
//{
//	CTaxasBaseRoomState::EnterState(pRoom);
//	m_pRoom->ResetRoomData() ;
//	m_pRoom->debugPlayerHistory();
//	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateWaitJoin");
//}
//
//void CTaxasStateWaitJoin::Update(float fDelte )
//{
//	if ( !m_pRoom->isRoomAlive() )
//	{
//		m_pRoom->GoToState(eRoomState_TP_Dead) ;
//		return ;
//	}
//
//	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitNextGame) >= 2 )
//	{
//		m_pRoom->GoToState(eRoomState_TP_BetBlind) ;
//	}
//}

// start blind bet state 
//void CTaxasStateBlindBet::EnterState(CTaxasRoom* pRoom )
//{
//	CTaxasBaseRoomState::EnterState(pRoom);
//	m_fDuringTime = TIME_BLIND_BET_STATE + 1.5f;
//	pRoom->StartGame();
//	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateBlindBet");
//}
//
//void CTaxasStateBlindBet::OnStateTimeOut()
//{
//	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
//	{
//		m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
//		return ;
//	}
//
//	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
//	{
//		m_pRoom->GoToState(eRoomState_TP_PrivateCard) ;
//	}
//	else
//	{
//		m_pRoom->GoToState(eRoomState_TP_OneRoundBetEndResult) ;
//	}
//}

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
				setStateDuringTime(TIME_PLAYER_BET_COIN_ANI + 0.2);
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

	uint8_t nWaitCaPlyCnt = m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
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
				uint64_t nValue = 0 ;
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
				CLogMgr::SharedLogMgr()->PrintLog("room id = %d player idx = %d do act = %d, value = %I64d",m_pRoom->getRoomID(), nSeatIdx,pAct->nPlayerAct,pAct->nValue ) ;
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
			setStateDuringTime(TIME_PLAYER_BET_COIN_ANI + 0.2);
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
	uint8_t nWaitCaPlyCnt = m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
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

	uint8_t nWaitCaPlyCnt = m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
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
    uint8_t nWaitCal = m_pRoom->getPlayerCntWithState(eRoomPeer_WaitCaculate);
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