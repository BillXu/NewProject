#include "TaxasRoomState.h"
#include "TaxasRoom.h"
#include "LogManager.h"
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
}

bool CTaxasStatePlayerBet::OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( CTaxasBaseRoomState::OnMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}
	return false ;
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