#include "TaxasRoom.h"
#include "LogManager.h"
#include "TaxasRoomState.h"
#include "TaxasServerApp.h"
#include "TaxasPokerPeerCard.h"
#include "ServerMessageDefine.h"
CTaxasRoom::CTaxasRoom()
{
	nRoomID = 0 ;
	memset(&m_stRoomConfig,0,sizeof(m_stRoomConfig));
	memset(m_vAllState,0,sizeof(m_vAllState)) ;
	m_eCurRoomState = eRoomState_TP_MAX ;
}

CTaxasRoom::~CTaxasRoom()
{
	for ( uint8_t nIdx = 0 ; nIdx < eRoomState_TP_MAX; ++nIdx )
	{
		if ( m_vAllState[nIdx] )
		{
			delete m_vAllState[nIdx] ;
			m_vAllState[nIdx] = NULL ;
		}
	}

	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin();
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		delete *iter ;
		*iter = NULL;
	}
	m_vAllPeers.clear() ;
	SetEnableUpdate(true) ;
}

bool CTaxasRoom::Init( uint32_t nRoomID,stTaxasRoomConfig* pRoomConfig )
{
	this->nRoomID = nRoomID ;
	m_nBankerIdx = 0;
	m_nLittleBlindIdx = 0;
	m_nBigBlindIdx = 0;
	m_nCurWaitPlayerActionIdx = 0;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nBetRound = 0 ;
	memset(m_vSitDownPlayers,0,sizeof(m_vSitDownPlayers)) ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}
	m_vAllPeers.clear();
	m_nLittleBlind = pRoomConfig->nBigBlind * 0.5 ;
	m_tPoker.InitTaxasPoker();
	if ( pRoomConfig->nMaxSeat > MAX_PEERS_IN_TAXAS_ROOM )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("config maxt is too big = %d",pRoomConfig->nMaxSeat ) ;
		pRoomConfig->nMaxSeat = MAX_PEERS_IN_TAXAS_ROOM ;
	}

	m_stRoomConfig = *pRoomConfig ;

	SetTimerManager(CTaxasServerApp::SharedGameServerApp()->getTimerMgr()) ;
	SetEnableUpdate(true) ;
	GoToState(eRoomState_TP_WaitJoin) ;
	return true ;
}

void CTaxasRoom::GoToState( eRoomState eState )
{
	if ( eState >= eRoomState_TP_MAX )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unknown target room state = %d",eState ) ;
		eState = eRoomState_TP_WaitJoin ;
	}

	if ( m_eCurRoomState < eRoomState_TP_MAX && m_vAllState[m_eCurRoomState] )
	{
		m_vAllState[m_eCurRoomState]->LeaveState();
	}

	m_eCurRoomState = eState ;
	// send msg to tell client ;
	stMsgTaxasRoomEnterState msgState ;
	msgState.fDuringTime = 0;//m_vAllState[m_eCurRoomState]->GetDuringTime() ;
	msgState.nNewState = m_eCurRoomState ;
	SendRoomMsg(&msgState,sizeof(msgState));

	if ( m_vAllState[m_eCurRoomState] == NULL )
	{
		m_vAllState[m_eCurRoomState] = CreateRoomState(m_eCurRoomState) ;
	}

	if ( m_vAllState[m_eCurRoomState] )
	{
		m_vAllState[m_eCurRoomState]->EnterState(this) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why cur state = %d is null" , m_eCurRoomState) ;
	}
}

void CTaxasRoom::Update( float fTimeElpas, unsigned int nTimerID )
{
	if ( m_vAllState[m_eCurRoomState] )
	{
		m_vAllState[m_eCurRoomState]->Update(fTimeElpas);
	}
}

CTaxasBaseRoomState* CTaxasRoom::CreateRoomState( eRoomState eState )
{
	CTaxasBaseRoomState* pState = NULL ;
	switch (eState)
	{
	case eRoomState_TP_WaitJoin:
		{
			pState = new CTaxasStateWaitJoin; 
		}
		break;
	case eRoomState_TP_BetBlind:
		{
			pState = new CTaxasStateBlindBet ;
		}
		break;
	case eRoomState_TP_PrivateCard:
		{
			pState = new CTaxasStatePrivateCard ;
		}
		break;
	case eRoomState_TP_Beting:
		{
			pState = new CTaxasStatePlayerBet ;
		}
		break;
	case eRoomState_TP_OneRoundBetEndResult:
		{
			pState = new CTaxasStateOneRoundBetEndResult ;
		}
		break;
	case eRoomState_TP_PublicCard:
		{
			pState = new CTaxasStatePublicCard ;
		}
		break;
	case eRoomState_TP_GameResult:
		{
			pState = new CTaxasStateGameResult ;
		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("create null room state id = %d ",eState ) ;
		return NULL ;
	}
	return pState ;
}

void CTaxasRoom::SendRoomMsg(stMsg* pMsg, uint16_t nLen )
{
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin();
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		stTaxasInRoomPeerData* pPeer = *iter ;
		if ( pPeer )
		{
			SendMsgToPlayer(pPeer->nSessionID,pMsg,nLen) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why have null peer in m_vAllPeers") ;
		}
	}
}

void CTaxasRoom::SendMsgToPlayer( uint32_t nSessionID, stMsg* pMsg, uint16_t nLen  )
{
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)pMsg,nLen) ;
}

bool CTaxasRoom::OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( m_vAllState[m_eCurRoomState] )
	{
		bool btru = m_vAllState[m_eCurRoomState]->OnMessage(prealMsg,eSenderPort,nPlayerSessionID);
		if ( btru )
		{
			return true ;
		}
	}
	return false ;

}

void CTaxasRoom::AddPlayer( stTaxasInRoomPeerDataExten& nPeerData )
{
	stTaxasInRoomPeerDataExten* pData = new stTaxasInRoomPeerDataExten ;
	memcpy(pData,&nPeerData,sizeof(stTaxasInRoomPeerDataExten));
	m_vAllPeers.push_back(pData);

	SendRoomInfoToPlayer(nPeerData.nSessionID);
}

bool CTaxasRoom::IsPlayerInRoomWithSessionID(uint32_t nSessionID )
{
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin() ;
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		if ( (*iter) && (*iter)->nSessionID == nSessionID  )
		{
			return true ;
		}
	}
	return false ;
}

void CTaxasRoom::OnPlayerSitDown(uint8_t nSeatIdx , uint32_t nSessionID , uint64_t nTakeInMoney )
{
	if ( nSeatIdx > m_stRoomConfig.nMaxSeat )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("error seat idx for session id = %d", nSeatIdx,nSessionID ) ;
		return ;
	}

	stTaxasInRoomPeerDataExten* pData = GetInRoomPlayerDataBySessionID(nSessionID) ;
	if ( pData == NULL || pData->IsHaveState(eRoomPeer_WithdrawingCoin) )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("nsession id = %d not in room cannot sit down or just withdrawing money ",nSessionID) ; 
		return ;
	}

	memcpy(&m_vSitDownPlayers[nSeatIdx],pData,sizeof(stTaxasInRoomPeerData));
	m_vSitDownPlayers[nSeatIdx].eCurAct = eRoomPeerAction_None ;
	m_vSitDownPlayers[nSeatIdx].nAllBetCoin = 0 ;
	m_vSitDownPlayers[nSeatIdx].nBetCoinThisRound = 0 ;
	m_vSitDownPlayers[nSeatIdx].nSeatIdx = nSeatIdx ;
	m_vSitDownPlayers[nSeatIdx].nTotalBuyInThisRoom = 0 ;
	m_vSitDownPlayers[nSeatIdx].nWinCoinThisGame = 0 ;
	memset(m_vSitDownPlayers[nSeatIdx].vHoldCard,0,sizeof(m_vSitDownPlayers[nSeatIdx].vHoldCard)) ;
	pData->nStateFlag &= (~eRoomPeer_StandUp);
	pData->nStateFlag |= eRoomPeer_SitDown;
	if ( pData->nCoinInRoom > nTakeInMoney )
	{
		m_vSitDownPlayers[nSeatIdx].nStateFlag = eRoomPeer_WaitNextGame ;
		m_vSitDownPlayers[nSeatIdx].nTakeInMoney = nTakeInMoney ;
		CLogMgr::SharedLogMgr()->PrintLog("takin coin enough uid = %d" ,pData->nUserUID);
	}
	else
	{
		m_vSitDownPlayers[nSeatIdx].nStateFlag = (eRoomPeer_WithdrawingCoin | eRoomPeer_SitDown );
		m_vSitDownPlayers[nSeatIdx].nTakeInMoney = pData->nCoinInRoom ;
		pData->nStateFlag |= eRoomPeer_WithdrawingCoin ;
		// not enough , request from data svr
		stMsgTaxasPlayerRequestCoin msgReq ;
		msgReq.bIsDiamond = false ;
		msgReq.nSessionID = nSessionID ;
		msgReq.nUserUID = pData->nUserUID ;
		msgReq.nWantMoney = nTakeInMoney - m_vSitDownPlayers[nSeatIdx].nTakeInMoney ;
		msgReq.nSeatIdx = nSeatIdx ;
		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msgReq,sizeof(msgReq)) ;
		CLogMgr::SharedLogMgr()->PrintLog("takin coin not enough request from data svr uid = %d" ,pData->nUserUID);
	}
	pData->nCoinInRoom -= m_vSitDownPlayers[nSeatIdx].nTakeInMoney ;


	// send msg tell other 
	stMsgTaxasRoomSitDown msgOther ;
	memcpy(&msgOther.tPlayerData,&m_vSitDownPlayers[nSeatIdx],sizeof(msgOther.tPlayerData));
	SendRoomMsg(&msgOther,sizeof(msgOther)) ;
}

void CTaxasRoom::OnPlayerStandUp(uint8_t nSeatIdx )
{
	if ( nSeatIdx >= m_stRoomConfig.nMaxSeat || m_vSitDownPlayers[nSeatIdx].IsInvalid() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("stand target idx = %d is invalid ,means no player in it ",nSeatIdx)  ;
		return ;
	}

	// first do give up action ; if you playing game ;
	if ( m_vSitDownPlayers[nSeatIdx].IsHaveState(eRoomPeer_WaitCaculate) )
	{
		//stMsgTaxasPlayerAct msg ;
		//msg.nValue = 0 ;
		//msg.nRoomID = GetRoomID() ;
		//msg.nPlayerAct = eRoomPeerAction_GiveUp ;
		//OnMessage(&msg,ID_MSG_PORT_CLIENT,m_vSitDownPlayers[m_nCurWaitPlayerActionIdx].nSessionID) ;
		uint64_t nValue = 0 ;
		OnPlayerAction(nSeatIdx,eRoomPeerAction_GiveUp,nValue);
	}

	// tell others 
	stMsgTaxasRoomStandUp msgOther ;
	msgOther.nSeatIdx = nSeatIdx ;
	SendRoomMsg(&msgOther,sizeof(msgOther)) ;

	// infor data serve the leave ;
	stTaxasInRoomPeerDataExten* pData = GetInRoomPlayerDataBySessionID(m_vSitDownPlayers[nSeatIdx].nSessionID) ;
	if ( pData == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("nsession id = %d will standup player not in room Big error, Big error !!",m_vSitDownPlayers[nSeatIdx].nSessionID ) ; 
		memset(&m_vSitDownPlayers[nSeatIdx],0,sizeof(m_vSitDownPlayers[nSeatIdx]) ) ;
		m_vSitDownPlayers[nSeatIdx].nSeatIdx = nSeatIdx ;
		return ;
	}

	if ( pData->IsHaveState(eRoomPeer_WithdrawingCoin) )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("you are withdraw coin ,why you standup uid = %d",pData->nUserUID );
	}

	// store back to CoinInRoom ;
	pData->nCoinInRoom += m_vSitDownPlayers[nSeatIdx].nTakeInMoney ;
	pData->nStateFlag |= eRoomPeer_StandUp ;
	pData->nStateFlag &= (~eRoomPeer_SitDown);

	// remove from seat ;
	memset(&m_vSitDownPlayers[nSeatIdx],0,sizeof(m_vSitDownPlayers[nSeatIdx]) ) ;
	m_vSitDownPlayers[nSeatIdx].nSeatIdx = nSeatIdx ;

}

uint8_t CTaxasRoom::GetSeatIdxBySessionID(uint32_t nSessionID )
{
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx )
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue;
		}

		if ( m_vSitDownPlayers[nIdx].nSessionID == nSessionID )
		{
			return nIdx ;
		}
	}

	return MAX_PEERS_IN_TAXAS_ROOM ;
}

void CTaxasRoom::OnPlayerLeaveRoom(uint32_t nPlayerSession )
{
	uint8_t nSeatIdx = GetSeatIdxBySessionID(nPlayerSession) ;
	if ( nSeatIdx < m_stRoomConfig.nMaxSeat )
	{
		OnPlayerStandUp(nSeatIdx) ;
	}

	// tell data server 
	if ( IsPlayerInRoomWithSessionID(nPlayerSession) )
	{
		stMsgInformTaxasPlayerLeave msgLeave ;
		msgLeave.nUserUID = GetInRoomPlayerDataBySessionID(nPlayerSession)->nUserUID ;
		msgLeave.nTakeInMoney = GetInRoomPlayerDataBySessionID(nPlayerSession)->nCoinInRoom ;
		msgLeave.bIsDiamond = false ;
		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msgLeave,sizeof(msgLeave)) ;
	}

	// remove from vec ;
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin() ;
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		if ( (*iter)->nSessionID == nPlayerSession )
		{
			delete (*iter) ;
			(*iter) = NULL ;
			m_vAllPeers.erase(iter) ;
			break; 
		}
	}
}

uint8_t CTaxasRoom::OnPlayerAction( uint8_t nSeatIdx ,eRoomPeerAction act , uint64_t& nValue )
{
	if ( nSeatIdx >= m_stRoomConfig.nMaxSeat || m_vSitDownPlayers[nSeatIdx].IsInvalid() )
	{
		return 2 ;
	}

	if ( nSeatIdx != m_nCurWaitPlayerActionIdx && act != eRoomPeerAction_GiveUp )
	{
		return 1 ;
	}

	if ( m_vSitDownPlayers[nSeatIdx].IsHaveState(eRoomPeer_CanAct) == false )
	{
		return 3 ;
	}

	stTaxasPeerData& pData = m_vSitDownPlayers[nSeatIdx] ;
	switch ( act )
	{
	case eRoomPeerAction_GiveUp:
		{
			m_nCurMainBetPool += pData.nBetCoinThisRound ;
			pData.eCurAct = eRoomPeerAction_GiveUp ;
			pData.nStateFlag = eRoomPeer_GiveUp ;
			// remove from vice pool
			for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
			{
				if ( m_vAllVicePools[nIdx].bUsed == false )
				{
					break;
				}
				m_vAllVicePools[nIdx].RemovePlayer(nSeatIdx) ;
			}
		}
		break;
	case eRoomPeerAction_Follow:
		{
			if ( pData.nTakeInMoney + pData.nBetCoinThisRound < m_nMostBetCoinThisRound )
			{
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,pData.nTakeInMoney);
			}
			pData.eCurAct = act ;
			pData.BetCoin(m_nMostBetCoinThisRound - pData.nBetCoinThisRound ) ;
		}
		break;
	case eRoomPeerAction_Add:
		{
			if ( pData.nTakeInMoney < nValue )
			{
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,pData.nTakeInMoney);
			}

			if ( pData.nBetCoinThisRound + nValue < m_nMostBetCoinThisRound + m_nLittleBlind * 2  )
			{
				return 6 ;
			}

			pData.eCurAct = act ;
			pData.BetCoin(nValue) ;
			m_nMostBetCoinThisRound = pData.nBetCoinThisRound ;
		}
		break;
	case eRoomPeerAction_AllIn:
		{
			pData.eCurAct = act ;
			pData.nStateFlag = eRoomPeer_AllIn;
			nValue = pData.nTakeInMoney ;
			pData.BetCoin(nValue) ;
			if ( pData.nBetCoinThisRound == 0 )
			{
				pData.nBetCoinThisRound = 1 ;   // avoid 0 all In bug ;
				CLogMgr::SharedLogMgr()->SystemLog("room id = %d , 0 coin all in player idx = %d",GetRoomID(),nSeatIdx) ;
			}

			if ( pData.nBetCoinThisRound > m_nMostBetCoinThisRound )
			{
				m_nMostBetCoinThisRound = pData.nBetCoinThisRound ;
			}
		}
		break;
	case eRoomPeerAction_Pass:
		{
			if ( m_nMostBetCoinThisRound != pData.nBetCoinThisRound )
			{
				return 5 ;
			}

			pData.eCurAct = eRoomPeerAction_Pass ;
		}
		break;
	default:
		return 4 ;
	}

	stMsgTaxasRoomAct msgOtherAct ;
	msgOtherAct.nPlayerAct = act ;
	msgOtherAct.nPlayerIdx = nSeatIdx ;
	msgOtherAct.nValue = nValue ;
	SendRoomMsg(&msgOtherAct,sizeof(msgOtherAct)) ;
	CLogMgr::SharedLogMgr()->PrintLog("player do act") ;
	return 0 ;
}

stTaxasPeerData* CTaxasRoom::GetSitDownPlayerData(uint8_t nSeatIdx)
{
	if ( nSeatIdx >= MAX_PEERS_IN_TAXAS_ROOM )
	{
		return nullptr ;
	}

	return &m_vSitDownPlayers[nSeatIdx];
}
// logic function 
uint8_t CTaxasRoom::GetPlayerCntWithState(eRoomPeerState eState )
{
	uint8_t nCnt = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue; 
		}

		if ( m_vSitDownPlayers[nIdx].IsHaveState(eState) )
		{
			++nCnt ;
		}
	}
	return nCnt ;
}

void CTaxasRoom::StartGame()
{
	// parepare all players ;
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue; 
		}

		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
		pData.nAllBetCoin = 0 ;
		pData.nBetCoinThisRound = 0 ;
		pData.nWinCoinThisGame = 0 ;
		if (pData.IsHaveState(eRoomPeer_WaitNextGame) )
		{
			pData.nStateFlag = eRoomPeer_CanAct ;
		}

		pData.eCurAct = eRoomPeerAction_None ;
		memset(pData.vHoldCard,0,sizeof(pData.vHoldCard)) ;
		CLogMgr::SharedLogMgr()->PrintLog("player uid = %d , left coin = %I64d",pData.nUserUID,pData.nTakeInMoney);
	}

	// prepare running data 
	m_nCurWaitPlayerActionIdx = -1;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nBetRound = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}
	m_tPoker.RestAllPoker();

	// init running data 
	m_nBankerIdx = GetFirstInvalidIdxWithState(m_nBankerIdx + 1 , eRoomPeer_CanAct) ;
	m_nLittleBlindIdx = GetFirstInvalidIdxWithState(m_nBankerIdx + 1 , eRoomPeer_CanAct) ;
	m_nBigBlindIdx = GetFirstInvalidIdxWithState(m_nLittleBlindIdx + 1 , eRoomPeer_CanAct) ;

	// bet coin this 
	m_vSitDownPlayers[m_nLittleBlindIdx].BetCoin( m_nLittleBlind ) ;
	m_vSitDownPlayers[m_nBigBlindIdx].BetCoin( m_nLittleBlind * 2 ) ;
	m_nMostBetCoinThisRound = m_nLittleBlind * 2 ;

	stMsgTaxasRoomStartRound msgStart ;
	msgStart.nBankerIdx = m_nBankerIdx ;
	msgStart.nBigBlindIdx = m_nBigBlindIdx ;
	msgStart.nLittleBlindIdx = m_nLittleBlindIdx ;
	SendRoomMsg(&msgStart,sizeof(msgStart));
}

void CTaxasRoom::ResetRoomData()
{
	// parepare all players ;
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue; 
		}

		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
		pData.nAllBetCoin = 0 ;
		pData.nBetCoinThisRound = 0 ;
		pData.nWinCoinThisGame = 0 ;
		pData.eCurAct = eRoomPeerAction_None ;
		memset(pData.vHoldCard,0,sizeof(pData.vHoldCard)) ;

		if ( eRoomPeer_LackOfCoin == pData.nStateFlag )
		{
			// second time player still have enough coin , should stand up
			CLogMgr::SharedLogMgr()->PrintLog("player lack of money standup uid = %d",pData.nUserUID);
			OnPlayerStandUp(nIdx);
			continue;
		}

		if (pData.nTakeInMoney < m_nLittleBlind * 2 )
		{
			if ( !pData.IsHaveState(eRoomPeer_WithdrawingCoin) )
			{
				pData.nStateFlag = eRoomPeer_LackOfCoin ;
				CLogMgr::SharedLogMgr()->PrintLog("player lack of money uid = %d ",pData.nUserUID);
			}
		}
		else
		{
			pData.nStateFlag = eRoomPeer_WaitNextGame ;
		}
	}

	// prepare running data 
	m_nCurWaitPlayerActionIdx = -1;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nBetRound = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}
}

void CTaxasRoom::DistributePrivateCard()
{
	stMsgTaxasRoomPrivateCard msgPrivate ;
	msgPrivate.nPlayerCnt = GetPlayerCntWithState(eRoomPeer_CanAct);
	uint16_t nBuferLen = sizeof(msgPrivate) + sizeof(stTaxasHoldCardItems) * msgPrivate.nPlayerCnt;
	char* pBuffer = new char[nBuferLen] ;
	uint16_t nOffset = 0 ;
	memcpy(pBuffer,&msgPrivate,sizeof(msgPrivate));
	nOffset += sizeof(msgPrivate);

	for ( uint8_t nCardIdx = 0 ; nCardIdx < TAXAS_PEER_CARD ; ++nCardIdx )
	{
		for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
		{
			if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_CanAct) == false ) )
			{
				continue; 
			}
			m_vSitDownPlayers[nIdx].vHoldCard[nCardIdx] = m_tPoker.GetCardWithCompositeNum() ;
			
			// the last card then copy to msg ;
			if ( TAXAS_PEER_CARD - 1 == nCardIdx )
			{
				stTaxasHoldCardItems privateCards ;
				privateCards.cPlayerIdx = nIdx ;
				privateCards.vCards[0] = m_vSitDownPlayers[nIdx].vHoldCard[0];
				privateCards.vCards[1] = m_vSitDownPlayers[nIdx].vHoldCard[1];
				memcpy(pBuffer + nOffset , &privateCards,sizeof(privateCards) );
				nOffset += sizeof(privateCards);
			}
		}
	}

	if ( nOffset != nBuferLen )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("buffer error for private cards") ;
		return ;
	}
	SendRoomMsg((stMsg*)pBuffer,nBuferLen) ;
	delete[] pBuffer ;
	pBuffer = NULL ;
}

void CTaxasRoom::PreparePlayersForThisRoundBet()
{
	if ( m_nCurWaitPlayerActionIdx >= 0 )  // means not first round 
	{
		m_nCurWaitPlayerActionIdx = m_nLittleBlindIdx - 1 ; //  little blid begin act   ps: m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState( m_nCurWaitPlayerActionIdx + 1 ,eRoomPeer_CanAct) ;
		m_nMostBetCoinThisRound = 0 ;
	}
	else
	{ 
		// first bet round do nothing 
		return ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_StayThisRound) == false ) )
		{
			continue; 
		}

		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
		pData.nBetCoinThisRound = 0 ;
		pData.eCurAct = eRoomPeerAction_None ;
	}
}

uint8_t CTaxasRoom::InformPlayerAct()
{
	if ( m_nCurWaitPlayerActionIdx < 0 ) // first round 
	{
		m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState(m_nBigBlindIdx + 1 ,eRoomPeer_CanAct) ;
	}
	else
	{
		m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState( m_nCurWaitPlayerActionIdx + 1 ,eRoomPeer_CanAct) ;
	}

	stMsgTaxasRoomWaitPlayerAct msgWait ;
	msgWait.nActPlayerIdx = m_nCurWaitPlayerActionIdx ;
	SendRoomMsg(&msgWait,sizeof(msgWait));
	CLogMgr::SharedLogMgr()->PrintLog("wait idx = %d act ",GetRoomID(),m_nCurWaitPlayerActionIdx ) ;
	return m_nCurWaitPlayerActionIdx ;
}

void CTaxasRoom::OnPlayerActTimeOut()
{
	stMsgTaxasPlayerAct msg ;
	msg.nValue = 0 ;
	msg.nRoomID = GetRoomID() ;
	if ( m_vSitDownPlayers[m_nCurWaitPlayerActionIdx].IsInvalid() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why cur wait player is null ");
		GoToState(eRoomState_TP_WaitJoin) ;
		return ;
	}

	if ( m_nMostBetCoinThisRound == m_vSitDownPlayers[m_nCurWaitPlayerActionIdx].nBetCoinThisRound )
	{
		msg.nPlayerAct = eRoomPeerAction_Pass ;
	}
	else
	{
		msg.nPlayerAct = eRoomPeerAction_GiveUp ;
	}
	OnMessage(&msg,ID_MSG_PORT_CLIENT,m_vSitDownPlayers[m_nCurWaitPlayerActionIdx].nSessionID) ;
}

bool CTaxasRoom::IsThisRoundBetOK()
{
	if ( GetPlayerCntWithState(eRoomPeer_WaitCaculate) <= 1 )
	{
		return true ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue; 
		}

		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
		
		if ( pData.IsHaveState(eRoomPeer_CanAct) && (pData.eCurAct == eRoomPeerAction_None || pData.nBetCoinThisRound != m_nMostBetCoinThisRound ) )
		{
			return false ;
		}
	}

	return true ;
}

 // return produced vice pool cunt this round ;
uint8_t CTaxasRoom::CaculateOneRoundPool()
{
	// check build vice pool
	uint8_t nBeforeVicePoolIdx = GetFirstCanUseVicePool().nIdx ;
	uint64_t nVicePool = 0 ;
	while ( true )
	{
		// find maybe pool 
		nVicePool = 0 ;
		for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx )
		{
			if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_WaitCaculate) == false ) )
			{
				continue;
			}

			stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
			if ( pData.eCurAct == eRoomPeerAction_AllIn && pData.nBetCoinThisRound > 0 )
			{
				if ( pData.nBetCoinThisRound < nVicePool || nVicePool == 0 )
				{
					nVicePool = pData.nBetCoinThisRound ;
				}
			}
		}

		if ( nVicePool == 0 )
		{
			break;
		}

		// real build pool;
		stVicePool& pPool = GetFirstCanUseVicePool();
		pPool.bUsed = true ;
		pPool.nCoin = m_nCurMainBetPool ;
		m_nCurMainBetPool = 0 ;
		
		// put player idx in pool ;
		CLogMgr::SharedLogMgr()->PrintLog("build pool pool idx = %d",pPool.nIdx ) ;
		for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx )
		{
			if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_WaitCaculate) == false ) )
			{
				continue;
			}

			stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
			if ( pData.nBetCoinThisRound > 0 )
			{
				pPool.nCoin += nVicePool ;
				pData.nBetCoinThisRound -= nVicePool ;
				pPool.vInPoolPlayerIdx.push_back(nIdx) ;
				CLogMgr::SharedLogMgr()->PrintLog("put player into pool player Idx = %d, UID = %d",nIdx,pData.nUserUID ) ;
			}
		}
		CLogMgr::SharedLogMgr()->PrintLog("pool idx = %d : coin = %I64d",pPool.nIdx,pPool.nCoin) ;
	}

	// build mian pool ;
	CLogMgr::SharedLogMgr()->PrintLog("build main pool: " ) ;
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx )
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_CanAct) == false ) )
		{
			continue;
		}

		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
		if ( pData.nBetCoinThisRound > 0 )
		{
			m_nCurMainBetPool += pData.nBetCoinThisRound ;
			CLogMgr::SharedLogMgr()->PrintLog("put player into Main pool player Idx = %d, UID = %d",nIdx,pData.nUserUID ) ;
		}
	}

	uint8_t nProducedVicePoolCnt = GetFirstCanUseVicePool().nIdx - nBeforeVicePoolIdx;
	CLogMgr::SharedLogMgr()->SystemLog("oneRound Caculate over, mainPool = %I64d, newVicePool = %d",m_nCurMainBetPool,nProducedVicePoolCnt );

	// send msg tell client [ nBeforeVicePoolIdx, GetFirstCanUseVicePoolIdx() ); this set of pool idx are new produced ; not include the last 
	stMsgTaxasRoomOneBetRoundResult msgResult ;
	msgResult.nCurMainPool = m_nCurMainBetPool ;
	msgResult.nNewVicePoolCnt = nProducedVicePoolCnt ;
	memset(msgResult.vNewVicePool,0,sizeof(msgResult.vNewVicePool)) ;
	for ( uint8_t nIdx = nBeforeVicePoolIdx, nNewIdx = 0 ; nIdx < GetFirstCanUseVicePool().nIdx; ++nIdx )
	{
		msgResult.vNewVicePool[nNewIdx++] = m_vAllVicePools[nIdx].nCoin ;
	}
	SendRoomMsg(&msgResult,sizeof(msgResult)) ;
	return nProducedVicePoolCnt ;
}

// return dis card cnt ;
uint8_t CTaxasRoom::DistributePublicCard()
{
	stMsgTaxasRoomPublicCard msgPublicCard ;
	// distr 3 
	if ( m_vPublicCardNums[0] == 0 )
	{
		msgPublicCard.nCardSeri = 0 ;
		for ( uint8_t nIdx = 0 ; nIdx < 3 ; ++nIdx )
		{
			m_vPublicCardNums[nIdx] = m_tPoker.GetCardWithCompositeNum() ;
			msgPublicCard.vCard[nIdx] = m_vPublicCardNums[nIdx] ;
		}
		// send msg to tell client ;
		SendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 3 ;
	}

	if ( m_vPublicCardNums[3] == 0 )
	{
		m_vPublicCardNums[3] = m_tPoker.GetCardWithCompositeNum() ;
		// send msg to tell client ;
		msgPublicCard.nCardSeri = 1 ;
		msgPublicCard.vCard[0] = m_vPublicCardNums[3] ;
		SendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 1 ;
	}

	if ( m_vPublicCardNums[4] == 0 )
	{
		m_vPublicCardNums[4] = m_tPoker.GetCardWithCompositeNum() ;
		// send msg to tell client ;
		msgPublicCard.nCardSeri = 2 ;
		msgPublicCard.vCard[0] = m_vPublicCardNums[4] ;
		SendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 1 ;
	}

	CLogMgr::SharedLogMgr()->ErrorLog("already finish public card why one more time ") ;
	return 0 ;
}

//return pool cnt ;
uint8_t CTaxasRoom::CaculateGameResult()
{
	// build a main pool;
	if ( m_nCurMainBetPool > 0 )
	{
		stVicePool& pPool = GetFirstCanUseVicePool();
		pPool.nCoin = m_nCurMainBetPool ;
		pPool.bUsed = true ;
		for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx )
		{
			stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
			if ( pData.IsInvalid() )
			{
				continue;
			}

			if ( pData.IsHaveState(eRoomPeer_CanAct) )
			{
				pPool.vInPoolPlayerIdx.push_back(nIdx) ;
			}
		}
	}

	// cacluate a main pool ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
	{
		if ( m_vAllVicePools[nIdx].bUsed )
		{
			CaculateVicePool(m_vAllVicePools[nIdx]) ;
		}
	}

	// send msg tell client ;
	if ( GetFirstCanUseVicePool().nIdx == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this game have no pool ? at least should have one room id = %d",nRoomID ) ;
		return 0 ;
	}

	uint8_t nLastPoolIdx = GetFirstCanUseVicePool().nIdx ;
	for ( uint8_t nIdx = 0 ; nIdx < nLastPoolIdx; ++nIdx )
	{
		stVicePool& pool = m_vAllVicePools[nIdx] ;
		if ( pool.vWinnerIdxs.empty() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("this pool have no winners , coin = %I64d, room = %d ",pool.nCoin,GetRoomID() ) ;
			continue;
		}

		stMsgTaxasRoomGameResult msgResult ;
		msgResult.nCoinPerWinner = pool.nCoin / pool.vWinnerIdxs.size() ;
		msgResult.nPoolIdx = nIdx ;
		msgResult.nWinnerCnt = 0;
		msgResult.bIsLastOne = (nIdx + 1) >= nLastPoolIdx ;
		VEC_INT8::iterator iter = pool.vWinnerIdxs.begin() ;
		for ( ; iter != pool.vWinnerIdxs.end() ; ++iter )
		{
			msgResult.vWinnerIdx[msgResult.nWinnerCnt++] = (*iter); 
		}
		SendRoomMsg(&msgResult,sizeof(msgResult)) ;
	}
	return GetFirstCanUseVicePool().nIdx ;
}

uint64_t CTaxasRoom::GetAllBetCoinThisRound()
{
	uint64_t nCoinThis = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_StayThisRound) == false ) )
		{
			continue; 
		}

		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
		nCoinThis += pData.nBetCoinThisRound;
	}
	return nCoinThis ;
}

bool CTaxasRoom::IsPublicDistributeFinish()
{
	return (m_vPublicCardNums[4] != 0 );
}

bool CTaxasRoom::isPlayerAlreadySitDown(uint32_t nSessionID )
{
	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx )
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue;
		}

		if ( m_vSitDownPlayers[nIdx].nSessionID == nSessionID  )
		{
			return true ;
		}
	}
	return false ;
}

uint8_t CTaxasRoom::GetFirstInvalidIdxWithState( uint8_t nIdxFromInclude , eRoomPeerState estate )
{
	for ( uint8_t nIdx = nIdxFromInclude ; nIdx < m_stRoomConfig.nMaxSeat * 2 ; ++nIdx )
	{
		uint8_t nRealIdx = nIdx % m_stRoomConfig.nMaxSeat ;
		if ( m_vSitDownPlayers[nRealIdx].IsInvalid() )
		{
			continue;
		}

		if ( m_vSitDownPlayers[nRealIdx].IsHaveState(estate) )
		{
			return nRealIdx ;
		}
	}
	CLogMgr::SharedLogMgr()->ErrorLog("why don't have peer with state = %d",estate ) ;
	return 0 ;
}

stVicePool& CTaxasRoom::GetFirstCanUseVicePool()
{
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
	{
		if ( !m_vAllVicePools[nIdx].bUsed )
		{
			 return m_vAllVicePools[nIdx] ;
		}
	}
	CLogMgr::SharedLogMgr()->ErrorLog("why all vice pool was used ? error ") ;
	return m_vAllVicePools[MAX_PEERS_IN_TAXAS_ROOM-1] ;
}

void CTaxasRoom::CaculateVicePool(stVicePool& pPool )
{
	if ( pPool.nCoin == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this pool coin is 0 ? players = %d room id = %d ",pPool.vInPoolPlayerIdx.size(),nRoomID ) ;
		return ;
	}

	if ( pPool.vInPoolPlayerIdx.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why pool coin = %I64d , peers is 0 room id = %d  ",pPool.nCoin,nRoomID ) ;
	}

	// find winner ;
	if ( pPool.vInPoolPlayerIdx.size() == 1 )
	{
		uint8_t nPeerIdx = pPool.vInPoolPlayerIdx[0] ;
		if ( m_vSitDownPlayers[nPeerIdx].IsInvalid() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this winner idx is invalid = %d, system got coin = %I64d",nPeerIdx,pPool.nCoin ) ;
			return ;
		}

		if ( m_vSitDownPlayers[nPeerIdx].IsHaveState(eRoomPeer_WaitCaculate) == false )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this winner idx state is invalid = %d, system got coin = %I64d",nPeerIdx,pPool.nCoin ) ;
			return ;
		}
		pPool.vWinnerIdxs.push_back( nPeerIdx ) ;
		m_vSitDownPlayers[nPeerIdx].nTakeInMoney += pPool.nCoin ;
		m_vSitDownPlayers[nPeerIdx].nWinCoinThisGame += pPool.nCoin ;
		return ;
	}

	// pk card
	if ( IsPublicDistributeFinish() == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("public is not finish how to pk card ? error room id = %d",nRoomID);
		return ;
	}

	CTaxasPokerPeerCard cardWinner ;
	for ( uint8_t nIdx = 0 ; nIdx < pPool.vInPoolPlayerIdx.size(); ++nIdx )
	{
		uint8_t nPeerIdx = pPool.vInPoolPlayerIdx[0] ;
		stTaxasPeerData& pData = m_vSitDownPlayers[nPeerIdx];
		if ( pData.IsInvalid() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this player in pool idx is invalid = %d, pool idx  = %I64d",nPeerIdx,pPool.nIdx ) ;
			continue; ;
		}

		if ( pData.IsHaveState(eRoomPeer_WaitCaculate) == false )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this player in pool state is invalid = %d, pool idx  = %I64d",nPeerIdx,pPool.nIdx ) ;
			continue ;
		}

		if ( pData.vHoldCard[0] == 0 || 0 == pData.vHoldCard[1] )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why peer idx = %d , uid = %d in pool hold card is invalid ",nPeerIdx,pData.nUserUID ) ;
			continue;
		}

		if ( pPool.vWinnerIdxs.empty() )
		{
			cardWinner.AddCardByCompsiteNum(pData.vHoldCard[0]);
			cardWinner.AddCardByCompsiteNum(pData.vHoldCard[1]);
			for ( uint8_t nPcardIdx = 0 ; nPcardIdx < TAXAS_PUBLIC_CARD ; ++nPcardIdx )
			{
				cardWinner.AddCardByCompsiteNum(m_vPublicCardNums[nPcardIdx]) ;
			}
			pPool.vWinnerIdxs.push_back(nPeerIdx) ;
			continue;
		}

		CTaxasPokerPeerCard curPeer ;
		curPeer.AddCardByCompsiteNum(pData.vHoldCard[0]);
		curPeer.AddCardByCompsiteNum(pData.vHoldCard[1]);
		for ( uint8_t nPcardIdx = 0 ; nPcardIdx < TAXAS_PUBLIC_CARD ; ++nPcardIdx )
		{
			curPeer.AddCardByCompsiteNum(m_vPublicCardNums[nPcardIdx]) ;
		}
		
		int8_t nRet = cardWinner.PK(&curPeer) ;
		if ( nRet < 0 )
		{
			pPool.vWinnerIdxs.clear();
			pPool.vWinnerIdxs.push_back(nPeerIdx) ;

			// switch winner 
			cardWinner.Reset();
			cardWinner.AddCardByCompsiteNum(pData.vHoldCard[0]);
			cardWinner.AddCardByCompsiteNum(pData.vHoldCard[1]);
			for ( uint8_t nPcardIdx = 0 ; nPcardIdx < TAXAS_PUBLIC_CARD ; ++nPcardIdx )
			{
				cardWinner.AddCardByCompsiteNum(m_vPublicCardNums[nPcardIdx]) ;
			}
		}
		else if ( nRet == 0 ) 
		{
			pPool.vWinnerIdxs.push_back(nPeerIdx) ;
		}
	}

	// give coin 
	if ( pPool.vWinnerIdxs.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why room id = %d pool idx = %d winner is empty , system got coin = %I64d ",nRoomID,pPool.nIdx,pPool.nCoin ) ;
		return ;
	}

	uint8_t nElasCoin = uint8_t(pPool.nCoin % pPool.vWinnerIdxs.size()) ;
	pPool.nCoin -= nElasCoin ;
	if ( nElasCoin > 0 )
	{
		CLogMgr::SharedLogMgr()->PrintLog("system got the elaps coin = %d, room id = %d , pool idx = %d ",nElasCoin,nRoomID,pPool.nIdx ) ;
	}
	uint64_t nCoinPerWinner = pPool.nCoin / pPool.vWinnerIdxs.size() ;
	for ( uint8_t nIdx = 0 ; nIdx < pPool.vWinnerIdxs.size(); ++nIdx )
	{
		stTaxasPeerData& pData = m_vSitDownPlayers[pPool.vWinnerIdxs[nIdx]];
		pData.nTakeInMoney += nCoinPerWinner ;
		pData.nWinCoinThisGame += nCoinPerWinner ;
		CLogMgr::SharedLogMgr()->PrintLog("player use uid = %d win coin = %I64d , from pool idx = %d, room id = %d",pData.nUserUID,nCoinPerWinner,pPool.nIdx,nRoomID) ;
	}
}

void CTaxasRoom::SendRoomInfoToPlayer(uint32_t nSessionID )
{
	// send base info 
	stMsgTaxasRoomInfoBase msgBaseInfo ;
	msgBaseInfo.eCurRoomState = m_eCurRoomState ;
	msgBaseInfo.nBankerIdx = m_nBankerIdx ;
	msgBaseInfo.nBigBlindIdx = m_nBigBlindIdx ;
	msgBaseInfo.nCurMainBetPool = m_nCurMainBetPool;
	msgBaseInfo.nCurWaitPlayerActionIdx = m_nCurWaitPlayerActionIdx ;
	msgBaseInfo.nLittleBlind = m_nLittleBlind ;
	msgBaseInfo.nLittleBlindIdx = m_nLittleBlindIdx ;
	msgBaseInfo.nMaxSeat = (uint8_t)m_stRoomConfig.nMaxSeat;
	msgBaseInfo.nMostBetCoinThisRound = m_nMostBetCoinThisRound ;
	msgBaseInfo.nRoomID = nRoomID ;
	msgBaseInfo.nMiniTakeIn = m_stRoomConfig.nMinNeedToEnter ;
	msgBaseInfo.nMaxTakeIn = m_stRoomConfig.nMaxTakeInCoin ;
	memcpy(msgBaseInfo.vPublicCardNums,m_vPublicCardNums,sizeof(msgBaseInfo.vPublicCardNums));
	SendMsgToPlayer(nSessionID,&msgBaseInfo,sizeof(msgBaseInfo)) ;

	// send vice pool 
	if ( GetFirstCanUseVicePool().nIdx )
	{
		stMsgTaxasRoomInfoVicePool msgVicePool ;
		memset(msgVicePool.vVicePool,0,sizeof(msgVicePool.vVicePool)) ;
		for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
		{
			msgVicePool.vVicePool[nIdx] = m_vAllVicePools[nIdx].nCoin;
		}
		SendMsgToPlayer(nSessionID,&msgVicePool,sizeof(msgVicePool));
	}

	// send Player data 
	uint8_t nCnt = GetPlayerCntWithState(eRoomPeer_SitDown) ;
	stMsgTaxasRoomInfoPlayerData msgPlayerData ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vSitDownPlayers[nIdx].IsInvalid() )
		{
			continue;
		}
		msgPlayerData.bIsLast = --nCnt > 0 ;
		memcpy(&msgPlayerData.tPlayerData,&m_vSitDownPlayers[nIdx],sizeof(msgPlayerData.tPlayerData));
		SendMsgToPlayer(nSessionID,&msgPlayerData,sizeof(msgPlayerData)) ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("send room data to player ");
}

stTaxasInRoomPeerDataExten* CTaxasRoom::GetInRoomPlayerDataBySessionID( uint32_t nSessionID )
{
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin();
	for ( ; iter != m_vAllPeers.end() ; ++iter )
	{
		if ( (*iter) && (*iter)->nSessionID == nSessionID  )
		{
			return (*iter) ;
		}
	}
	return NULL;
}

void CTaxasRoom::syncPlayerDataToDataSvr( uint32_t nSessionID )
{
	// if player requesting coin , do not sync data ;
	/// and just after game result ;
}
