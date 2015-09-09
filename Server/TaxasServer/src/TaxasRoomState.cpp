#include "TaxasRoomState.h"
#include "TaxasRoom.h"
#include "LogManager.h"
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include "TaxasServerApp.h"
CTaxasBaseRoomState::CTaxasBaseRoomState()
{
	m_fDuringTime = 0 ;
	m_pRoom = NULL ;
}

CTaxasBaseRoomState::~CTaxasBaseRoomState()
{

}

bool CTaxasBaseRoomState::OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch( prealMsg->usMsgType )
	{
	case MSG_TP_PLAYER_SIT_DOWN:
		{
			stMsgTaxasPlayerSitDown* pRet = (stMsgTaxasPlayerSitDown*)prealMsg ;
			stMsgTaxasPlayerSitDownRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.nSeatIdx = pRet->nSeatIdx ;
			if ( m_pRoom->IsPlayerInRoomWithSessionID(nPlayerSessionID) == false )
			{
				msgBack.nRet = 2;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("you are not in room session id = %d",nPlayerSessionID ) ;
				return true ;
			}

			if ( m_pRoom->isPlayerAlreadySitDown(nPlayerSessionID) )
			{
				msgBack.nRet = 4;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("you are already sit down session id = %d",nPlayerSessionID ) ;
				return true ;
			}

			if ( pRet->nSeatIdx >= m_pRoom->m_stRoomConfig.nMaxSeat || m_pRoom->m_vSitDownPlayers[pRet->nSeatIdx].IsInvalid() == false )
			{
				msgBack.nRet = 1;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->PrintLog("invalid seat idx = %d , can not sit down session id = %d",pRet->nSeatIdx,nPlayerSessionID ) ;
				return true ;
			}

			if ( pRet->nTakeInMoney > m_pRoom->m_stRoomConfig.nMaxTakeInCoin || pRet->nTakeInMoney < m_pRoom->m_stRoomConfig.nMiniTakeInCoin )
			{
				msgBack.nRet = 3;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->PrintLog("invalid take in coin = %I64d , can not sit down session id = %d",pRet->nTakeInMoney,nPlayerSessionID ) ;
				return true ;
			}

			stTaxasInRoomPeerDataExten* pData = m_pRoom->GetInRoomPlayerDataBySessionID(nPlayerSessionID);
			if ( pData == nullptr )
			{
				msgBack.nRet = 2;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("you are not in room session id = %d data is null",nPlayerSessionID ) ;
				return true ;
			}

			m_pRoom->OnPlayerSitDown(pRet->nSeatIdx,nPlayerSessionID,pRet->nTakeInMoney) ;
			CLogMgr::SharedLogMgr()->PrintLog("player seat idx = %d sit down want takein coin = %I64d",pRet->nSeatIdx,pRet->nTakeInMoney) ;
			return true ;
		}
		break;
	//case MSG_REQUEST_MONEY:
	//	{
	//		stTaxasInRoomPeerDataExten* pPlayrInRoomData = m_pRoom->GetInRoomPlayerDataBySessionID(nPlayerSessionID);
	//		//if ( pPlayrInRoomData )
	//		//{
	//		//	// as withdrawing coin come back , so remove the drawing coin state ;
	//		//	pPlayrInRoomData->nStateFlag &= (~eRoomPeer_WithdrawingCoin) ;
	//		//}
	//		
	//		stMsgPlayerRequestCoinRet* pRet = (stMsgPlayerRequestCoinRet*)prealMsg ;
	//		int8_t nSeatIdx = pRet->nBackArg[1];
	//		stMsgTaxasRoomUpdatePlayerState msgNewState ;
	//		if ( pRet->nRet )
	//		{
	//			// player still at seat when money arrived ;
	//			if ( nSeatIdx < m_pRoom->m_stRoomConfig.nMaxSeat && m_pRoom->m_vSitDownPlayers[nSeatIdx].IsInvalid() == false && m_pRoom->m_vSitDownPlayers[nSeatIdx].nUserUID == pRet->nUserUID  )
	//			{
	//				m_pRoom->OnPlayerStandUp(nSeatIdx);
	//			}
	//			CLogMgr::SharedLogMgr()->PrintLog(" withdrawing coin erro  = %d , uid = %d",pRet->nRet,pRet->nUserUID) ;
	//			
	//			if ( pPlayrInRoomData )
	//			{
	//				// still in room inform sit ret ;
	//				stMsgWithdrawingMoneyRet msgRet ;
	//				msgRet.nRet = 1 ;
	//				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
	//			}
	//		}
	//		else
	//		{
	//			// need tell data svr the result ;
	//			stMsgTaxasPlayerRequestCoinComfirm msgDataSvrBack ;
	//			msgDataSvrBack.bDiamond = pRet->bIsDiamond ;
	//			msgDataSvrBack.nUserUID = pRet->nUserUID ;
	//			msgDataSvrBack.nWantedMoney = pRet->nAddedMoney ;
	//			msgDataSvrBack.nRet = 0 ;
	//			
	//			if ( nSeatIdx > m_pRoom->m_stRoomConfig.nMaxSeat || m_pRoom->m_vSitDownPlayers[nSeatIdx].IsInvalid() || m_pRoom->m_vSitDownPlayers[nSeatIdx].nUserUID != pRet->nUserUID  )
	//			{
	//				msgDataSvrBack.nRet = 1 ;
	//				CLogMgr::SharedLogMgr()->ErrorLog("money arrived ,but you have gone standup = %d",nPlayerSessionID ) ;
	//				if ( pPlayrInRoomData )
	//				{
	//					// still in room inform sit ret ;
	//					stMsgWithdrawingMoneyRet msgRet ;
	//					msgRet.nRet = 2 ;
	//					m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
	//				}
	//			}
	//			else
	//			{
	//				m_pRoom->m_vSitDownPlayers[nSeatIdx].nStateFlag = eRoomPeer_WaitNextGame ;
	//				m_pRoom->m_vSitDownPlayers[nSeatIdx].nTakeInMoney += pRet->nAddedMoney ;
	//				m_pRoom->m_vSitDownPlayers[nSeatIdx].nTotalBuyInThisRoom += pRet->nAddedMoney ;

	//				msgNewState.nSeatIdx = nSeatIdx ;
	//				msgNewState.nStateFlag = eRoomPeer_WaitNextGame ;
	//				msgNewState.nTakeInCoin = m_pRoom->m_vSitDownPlayers[nSeatIdx].nTakeInMoney ;
	//	
	//				m_pRoom->SendRoomMsg(&msgNewState,sizeof(msgNewState));

	//				CLogMgr::SharedLogMgr()->PrintLog("withdraw coin ok uid = %d coin = %I64d",pRet->nUserUID,msgNewState.nTakeInCoin );
	//			}
	//			 // tell data server ;
	//			CLogMgr::SharedLogMgr()->ErrorLog("tell data svr request comfirm uid = %d",pRet->nUserUID);
	//			CTaxasServerApp::SharedGameServerApp()->sendMsg(m_pRoom->GetRoomID(),(char*)&msgDataSvrBack,sizeof(msgDataSvrBack)) ;
	//			return true ;
	//		}
	//	}
	//	break;
	case MSG_TP_PLAYER_STAND_UP:
		{
			uint8_t nSeatIdx = m_pRoom->GetSeatIdxBySessionID(nPlayerSessionID);
			if ( nSeatIdx >= m_pRoom->m_stRoomConfig.nMaxSeat )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("you already stand up session id = %d",nPlayerSessionID ) ;
				break;
			}
			m_pRoom->OnPlayerStandUp(nSeatIdx) ;
			CLogMgr::SharedLogMgr()->PrintLog("player seat idx = %d stand up ",nSeatIdx );
		}
		break;
	case MSG_TP_ORDER_LEAVE:
	case MSG_TP_PLAYER_LEAVE:
		{
			if ( m_pRoom->IsPlayerInRoomWithSessionID(nPlayerSessionID) == false )
			{
				// already check by top level code ;
				CLogMgr::SharedLogMgr()->ErrorLog("you are not in room so how you leave , session id = %d",nPlayerSessionID ) ;
				break;
			}
			m_pRoom->OnPlayerLeaveRoom(nPlayerSessionID) ;
			CLogMgr::SharedLogMgr()->PrintLog("player session id = %d leave room ",nPlayerSessionID ) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CTaxasBaseRoomState::EnterState(CTaxasRoom* pRoom )
{
	m_pRoom = pRoom ;
}

void CTaxasBaseRoomState::LeaveState()
{
	m_fDuringTime = 0 ;
}

void CTaxasBaseRoomState::Update(float fDelte )
{
	m_fDuringTime -= fDelte ;
	if ( m_fDuringTime <= 0 )
	{
		OnStateTimeOut();
	}
}

void CTaxasStateDead::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
}

void CTaxasStateDead::Update(float fDelte )
{
	if ( m_pRoom->isRoomAlive() )
	{
		m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
		return ;
	}
}

// wait join state 
void CTaxasStateWaitJoin::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_pRoom->ResetRoomData() ;
	m_pRoom->debugPlayerHistory();
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateWaitJoin");
}

void CTaxasStateWaitJoin::Update(float fDelte )
{
	if ( !m_pRoom->isRoomAlive() )
	{
		m_pRoom->GoToState(eRoomState_TP_Dead) ;
		return ;
	}

	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitNextGame) >= 2 )
	{
		m_pRoom->GoToState(eRoomState_TP_BetBlind) ;
	}
}

// start blind bet state 
void CTaxasStateBlindBet::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_fDuringTime = TIME_BLIND_BET_STATE + 1.5f;
	pRoom->StartGame();
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateBlindBet");
}

void CTaxasStateBlindBet::OnStateTimeOut()
{
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
	{
		m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
		return ;
	}

	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->GoToState(eRoomState_TP_PrivateCard) ;
	}
	else
	{
		m_pRoom->GoToState(eRoomState_TP_OneRoundBetEndResult) ;
	}
}

// private card
void CTaxasStatePrivateCard::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_fDuringTime = ( TIME_TAXAS_DISTRIBUTE_HOLD_CARD_DELAY * (m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) * 2 - 1) ) + TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD;
	if ( m_fDuringTime < 0 )
	{
		m_fDuringTime = 0 ;
		CLogMgr::SharedLogMgr()->ErrorLog("distribute hold card time < 0 peer = %d",m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct));
	}
	m_fDuringTime += 1.5 ;
	m_pRoom->DistributePrivateCard();
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStatePrivateCard");
}


void CTaxasStatePrivateCard::OnStateTimeOut()
{
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
	{
		m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
		return ;
	}

	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->GoToState(eRoomState_TP_Beting) ;
	}
	else
	{
		m_pRoom->GoToState(eRoomState_TP_OneRoundBetEndResult) ;
	}
}

// player bet state 
void CTaxasStatePlayerBet::EnterState(CTaxasRoom* pRoom )
{
	m_bHavePlayerActing = false ;
	m_fLeftActingTime = 0 ;
	m_bIsCurActPlayerActing = false ;

	CTaxasBaseRoomState::EnterState(pRoom);
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) < 2 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can act player < 2 should not enter bet state ") ;
		m_pRoom->GoToState(eRoomState_TP_OneRoundBetEndResult);
		return ;
	}
	m_pRoom->PreparePlayersForThisRoundBet();
	m_nCurActPlayerIdx = m_pRoom->InformPlayerAct();
	ResetStateTime();
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStatePlayerBet");
}

void CTaxasStatePlayerBet::Update(float fDelte )
{
	if ( m_bHavePlayerActing )
	{
		m_fLeftActingTime-= fDelte ;
		if ( m_fLeftActingTime <= 0 )
		{
			m_bHavePlayerActing = false ;
			m_fLeftActingTime = 0 ;
			PlayerDoActOver();
		}
	}
 
	if ( m_bIsCurActPlayerActing  == false )  // wait player  act . if cur player is acting , need not to timeOut ;
	{
		CTaxasBaseRoomState::Update(fDelte);
	}
}

void CTaxasStatePlayerBet::OnStateTimeOut()
{
	if ( m_bIsCurActPlayerActing )  
	{
		return ;
	}

	m_pRoom->OnPlayerActTimeOut() ;
	CLogMgr::SharedLogMgr()->PrintLog("wait time out");
	//if ( m_pRoom->IsThisRoundBetOK() == false )
	//{
	//	m_pRoom->InformPlayerAct();
	//	ResetStateTime();
	//}
}

void CTaxasStatePlayerBet::ResetStateTime()
{
	m_fDuringTime = TIME_TAXAS_BET + 2 ;
}

bool CTaxasStatePlayerBet::OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_TP_ORDER_LEAVE:
	case MSG_TP_PLAYER_LEAVE:
	case MSG_TP_PLAYER_STAND_UP:
		{
			uint8_t nSeatIdx = m_pRoom->GetSeatIdxBySessionID(nPlayerSessionID);
			if ( nSeatIdx >= MAX_PEERS_IN_TAXAS_ROOM )
			{
				CTaxasBaseRoomState::OnMessage(prealMsg,eSenderPort,nPlayerSessionID);
				return true ;
			}

			stTaxasPeerData* pData = m_pRoom->GetSitDownPlayerData(nSeatIdx);
			bool bNedWait = false ;
			uint64_t nBetCoinThisRound = 0 ;
			if ( pData != nullptr && pData->IsHaveState(eRoomPeer_WaitCaculate) )
			{
				bNedWait = true ;
				nBetCoinThisRound = pData->nBetCoinThisRound ;
			}
			CTaxasBaseRoomState::OnMessage(prealMsg,eSenderPort,nPlayerSessionID);

			if ( bNedWait )
			{
				float nThisActTime = nBetCoinThisRound > 0 ? TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL : 0 ;
				if ( m_bHavePlayerActing )
				{
					m_fLeftActingTime = m_fLeftActingTime > nThisActTime ? m_fLeftActingTime : nThisActTime ;
				}
				else
				{
					m_fLeftActingTime = nThisActTime;
				}
				m_bHavePlayerActing = true ;

				if ( m_pRoom->GetCurWaitActPlayerIdx() == nSeatIdx )
				{
					m_bIsCurActPlayerActing = true ;
				}
			}
		}
		break;
	case MSG_TP_PLAYER_ACT:
		{
			uint8_t nSeatIdx = m_pRoom->GetSeatIdxBySessionID(nPlayerSessionID);
			stMsgTaxasPlayerActRet msgBack ;
			stMsgTaxasPlayerAct* pAct = (stMsgTaxasPlayerAct*)prealMsg ;
			msgBack.nRet = m_pRoom->OnPlayerAction( nSeatIdx,(eRoomPeerAction)pAct->nPlayerAct,pAct->nValue) ;
			if ( msgBack.nRet == 0 )
			{
				float nThisActTime = TIME_PLAYER_BET_COIN_ANI + 0.7f;
				if ( pAct->nPlayerAct == eRoomPeerAction_GiveUp  )
				{
					nThisActTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL ;
				}
				else if ( eRoomPeerAction_Pass == pAct->nPlayerAct )
				{
					nThisActTime = nThisActTime * 0.5f ;
				}

				if ( m_bHavePlayerActing  )
				{
					m_fLeftActingTime = m_fLeftActingTime > nThisActTime ? m_fLeftActingTime : nThisActTime ;
				}
				else
				{
					m_fLeftActingTime = nThisActTime ;
				}

				m_bHavePlayerActing = true ;
				if ( nSeatIdx == m_pRoom->GetCurWaitActPlayerIdx()  )
				{
					 m_bIsCurActPlayerActing = true ;
				}
				CLogMgr::SharedLogMgr()->PrintLog("room id = %d player idx = %d do act = %d, value = %I64d",m_pRoom->GetRoomID(), nSeatIdx,pAct->nPlayerAct,pAct->nValue ) ;
			}
			else
			{
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack));
				CLogMgr::SharedLogMgr()->ErrorLog("player idx = %d act error ret = %d , room id = %d",nSeatIdx,msgBack.nRet,m_pRoom->GetRoomID()); 
			}
		}
		break;
	default:
		return CTaxasBaseRoomState::OnMessage(prealMsg,eSenderPort,nPlayerSessionID);
	}
	return true ;
}

void CTaxasStatePlayerBet::PlayerDoActOver()
{
	if ( m_pRoom->IsThisRoundBetOK() )
	{
		m_pRoom->GoToState(eRoomState_TP_OneRoundBetEndResult) ;
	}
	else 
	{
		if ( m_bIsCurActPlayerActing )
		{
			m_pRoom->InformPlayerAct() ;
			ResetStateTime() ;
			m_bIsCurActPlayerActing = false ;
		}
	}
}
// caculate vice pool
void CTaxasStateOneRoundBetEndResult::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	uint8_t nCnt = m_pRoom->CaculateOneRoundPool() ;
	if ( m_pRoom->GetAllBetCoinThisRound() <= 0 && nCnt == 0 )
	{
		m_fDuringTime = 1.2f ;
		CLogMgr::SharedLogMgr()->PrintLog("not bet just go over");
	}
	else
	{
		m_fDuringTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL + TIME_TAXAS_MAKE_VICE_POOLS * nCnt + 1.2f ; // if produced vice pool , need more time ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateOneRoundBetEndResult");
}

void CTaxasStateOneRoundBetEndResult::OnStateTimeOut()
{
	if ( m_pRoom->IsPublicDistributeFinish() )
	{
		m_pRoom->GoToState(eRoomState_TP_GameResult) ;
	}
	else
	{
		if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) >= 2 )
		{
			m_pRoom->GoToState(eRoomState_TP_PublicCard) ;
		}
		else if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("this round no player want coin , so go wait join");
			m_pRoom->GoToState(eRoomState_TP_WaitJoin);
		}
		else
		{
			m_pRoom->GoToState(eRoomState_TP_GameResult) ;
		}
	}
}

// public card
void CTaxasStatePublicCard::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_fDuringTime = TIME_DISTRIBUTE_ONE_PUBLIC_CARD * m_pRoom->DistributePublicCard() + 1.2f;
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStatePublicCard");
}

void CTaxasStatePublicCard::OnStateTimeOut()
{
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why all player leave , they do not like money ?");
		m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
		return;
	}

	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->GoToState(eRoomState_TP_Beting) ;
	}
	else
	{
		if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate) >= 2 && (m_pRoom->IsPublicDistributeFinish() == false ) )
		{
			m_pRoom->GoToState(eRoomState_TP_PublicCard) ;
		}
		else
		{
			m_pRoom->GoToState(eRoomState_TP_GameResult) ;
		}
	}
}

// game result 
void CTaxasStateGameResult::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
    uint8_t nWaitCal = m_pRoom->GetPlayerCntWithState(eRoomPeer_WaitCaculate);
	if ( nWaitCal > 1 )
	{
		m_fDuringTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + TIME_TAXAS_SHOW_BEST_CARD + 3;
	}
	else
	{
		m_fDuringTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + 1.3f;
	}
	
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateGameResult");
}


void CTaxasStateGameResult::OnStateTimeOut()
{
	m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
}