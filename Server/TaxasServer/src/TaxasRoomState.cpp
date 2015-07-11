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

			if ( pRet->nTakeInMoney > m_pRoom->m_stRoomConfig.nMaxTakeInCoin || pRet->nTakeInMoney < m_pRoom->m_stRoomConfig.nMinNeedToEnter )
			{
				msgBack.nRet = 3;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->PrintLog("invalid take in coin = %I64d , can not sit down session id = %d",pRet->nTakeInMoney,nPlayerSessionID ) ;
				return true ;
			}

			// request data serve to get coin ;
			stMsgRequestTaxasPlayerTakeInCoin msgTakein ;
			msgTakein.bIsDiamond = false ;
			msgTakein.nMoneyToTakeIn = pRet->nTakeInMoney ;
			msgTakein.nPlayerSessionID = nPlayerSessionID;
			msgTakein.nSeatIdx = pRet->nSeatIdx ;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(m_pRoom->GetRoomID(),(char*)&msgTakein,sizeof(msgTakein)) ;
			CLogMgr::SharedLogMgr()->PrintLog("request take in coin,session id = %d",nPlayerSessionID);
			CLogMgr::SharedLogMgr()->ErrorLog("can not be do twice , most do something avoid it");
			return true ;
		}
		break;
	case MSG_TP_REQUEST_TAKE_IN_MONEY:
		{
			stMsgRequestTaxasPlayerTakeInCoinRet* pRet = (stMsgRequestTaxasPlayerTakeInCoinRet*)prealMsg ;
			if ( pRet->nRet )
			{
				stMsgTaxasPlayerSitDownRet msgBack ;
				msgBack.nSeatIdx = pRet->nSeatIdx ;
				msgBack.nRet = 4;
				m_pRoom->SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->PrintLog("can not sit down sever say error = %d , session id = %d",pRet->nRet,nPlayerSessionID) ;
			}
			else
			{
				if ( pRet->nSeatIdx > m_pRoom->m_stRoomConfig.nMaxSeat || m_pRoom->m_vSitDownPlayers[pRet->nSeatIdx].IsInvalid() == false || m_pRoom->IsPlayerInRoomWithSessionID(nPlayerSessionID) == false  )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("argument error , can not sit down session id = %d",nPlayerSessionID ) ;

					stMsgInformTaxasPlayerStandUp msgUp ;
					msgUp.bIsDiamond = pRet->bIsDiamond ;
					msgUp.nTakeInMoney = pRet->nMoneyToTakeIn ;
					msgUp.nUserUID = 0 ;
					msgUp.nSessionID = nPlayerSessionID ;
					m_pRoom->SendMsgToPlayer(m_pRoom->nRoomID,&msgUp,sizeof(msgUp));  // tell data server ;

					return true ;
				}
				m_pRoom->OnPlayerSitDown(pRet->nSeatIdx,nPlayerSessionID,pRet->nMoneyToTakeIn) ;
				CLogMgr::SharedLogMgr()->PrintLog("player seat idx = %d sit down takein coin = %I64d",pRet->nSeatIdx,pRet->nMoneyToTakeIn) ;
				return true ;
			}
		}
		break;
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

// wait join state 
void CTaxasStateWaitJoin::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_pRoom->ResetRoomData() ;
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateWaitJoin");
}

void CTaxasStateWaitJoin::Update(float fDelte )
{
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_SitDown) >= 2 )
	{
		m_pRoom->GoToState(eRoomState_TP_BetBlind) ;
	}
}

// start blind bet state 
void CTaxasStateBlindBet::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_fDuringTime = TIME_BLIND_BET_STATE ;
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
 
	CTaxasBaseRoomState::Update(fDelte);
}

void CTaxasStatePlayerBet::OnStateTimeOut()
{
	m_pRoom->OnPlayerActTimeOut() ;
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

			stTaxasPeerData* pData = m_pRoom->GetSitDownPlayerData(nPlayerSessionID);
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
				float nThisActTime = TIME_PLAYER_BET_COIN_ANI ;
				if ( pAct->nPlayerAct == eRoomPeerAction_GiveUp  )
				{
					nThisActTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL ;
				}
				else if ( eRoomPeerAction_Pass == pAct->nPlayerAct )
				{
					nThisActTime = 0 ;
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
	if ( m_pRoom->GetAllBetCoinThisRound() <= 0 )
	{
		m_fDuringTime = 0 ;
	}
	else
	{
		m_fDuringTime = TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL + TIME_TAXAS_MAKE_VICE_POOLS * nCnt + 1 ; // if produced vice pool , need more time ;
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
	m_fDuringTime = TIME_DISTRIBUTE_ONE_PUBLIC_CARD * m_pRoom->DistributePublicCard() + 1;
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStatePublicCard");
}

void CTaxasStatePublicCard::OnStateTimeOut()
{
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
	m_fDuringTime = TIME_TAXAS_CACULATE_PER_BET_POOL * m_pRoom->CaculateGameResult() + 2;
	CLogMgr::SharedLogMgr()->PrintLog("CTaxasStateGameResult");
}


void CTaxasStateGameResult::OnStateTimeOut()
{
	m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
}