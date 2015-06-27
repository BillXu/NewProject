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
			CLogMgr::SharedLogMgr()->ErrorLog("player session id = %d leave room ",nPlayerSessionID ) ;
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
	m_fDuringTime = 3 ;
	pRoom->StartGame();
}

void CTaxasStateBlindBet::OnStateTimeOut()
{
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
	m_fDuringTime = 0.3 * m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) ;
	m_pRoom->DistributePrivateCard();
}


void CTaxasStatePrivateCard::OnStateTimeOut()
{
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) >= 2 )
	{
		m_pRoom->GoToState(eRoomState_TP_Beting) ;
	}
	else
	{
		m_pRoom->GoToState(eRoomState_TP_GameResult) ;
	}
}

// player bet state 
void CTaxasStatePlayerBet::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_fDuringTime = 15 ;
	if ( m_pRoom->GetPlayerCntWithState(eRoomPeer_CanAct) < 2 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can act player < 2 should enter bet state ") ;
	}
	m_pRoom->PreparePlayersForThisRoundBet();
	m_nCurActPlayerIdx = m_pRoom->InformPlayerAct();
}

void CTaxasStatePlayerBet::Update(float fDelte )
{
	CTaxasBaseRoomState::Update(fDelte);
	if ( m_pRoom->IsThisRoundBetOK() )
	{
		if ( m_pRoom->GetAllBetCoinThisRound() > 0 )
		{
			m_pRoom->GoToState(eRoomState_TP_OneRoundBetEndResult) ;
		}
		else
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
	}
}

void CTaxasStatePlayerBet::OnStateTimeOut()
{
	m_pRoom->OnPlayerActTimeOut() ;

	if ( m_pRoom->IsThisRoundBetOK() == false )
	{
		m_pRoom->InformPlayerAct();
	}
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
			CTaxasBaseRoomState::OnMessage(prealMsg,eSenderPort,nPlayerSessionID);
			if ( m_pRoom->GetCurWaitActPlayerIdx() == nSeatIdx )
			{
				if ( m_pRoom->IsThisRoundBetOK() == false )
				{
					m_pRoom->InformPlayerAct();
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
				CLogMgr::SharedLogMgr()->PrintLog("room id = %d player idx = %d do act = %d, value = %I64d",m_pRoom->GetRoomID(), nSeatIdx,pAct->nPlayerAct,pAct->nValue ) ;
				if ( nSeatIdx == m_pRoom->GetCurWaitActPlayerIdx() && m_pRoom->IsThisRoundBetOK() == false )
				{
					m_pRoom->InformPlayerAct();
				}
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

// caculate vice pool
void CTaxasStateOneRoundBetEndResult::EnterState(CTaxasRoom* pRoom )
{
	CTaxasBaseRoomState::EnterState(pRoom);
	m_fDuringTime = 1 + 0.8 * m_pRoom->CaculateOneRoundPool();  // if produced vice pool , need more time ;
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
	m_fDuringTime = 0.5 * m_pRoom->DistributePublicCard();
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
	m_fDuringTime = 0.5 * m_pRoom->CaculateGameResult();
}


void CTaxasStateGameResult::OnStateTimeOut()
{
	m_pRoom->GoToState(eRoomState_TP_WaitJoin) ;
}