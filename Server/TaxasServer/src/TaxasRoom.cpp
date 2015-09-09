#include "TaxasRoom.h"
#include "LogManager.h"
#include "TaxasRoomState.h"
#include "TaxasServerApp.h"
#include "TaxasPokerPeerCard.h"
#include "ServerMessageDefine.h"
#include <json/json.h>
#include "AutoBuffer.h"
#define TIME_SECONDS_PER_DAY (60*60*24)
#define TIME_SAVE_ROOM_INFO 60*30
CTaxasRoom::CTaxasRoom()
{
	nRoomID = 0 ;
	memset(&m_stRoomConfig,0,sizeof(m_stRoomConfig));
	memset(m_vAllState,0,sizeof(m_vAllState)) ;
	m_eCurRoomState = eRoomState_TP_MAX ;

	m_nRoomOwnerUID = 0 ;
	m_nCreateTime = 0 ;
	m_nDeadTime = 0 ;
	m_nAvataID = 0 ;
	memset(m_vRoomName,0,MAX_LEN_ROOM_NAME);
	m_strRoomDesc = "" ;
	m_strRoomInForm = "" ;
	m_nRoomProfit = 0 ;
	m_nInformSerial = 0 ;
	m_bRoomInfoDirty = false ;
	m_TimeSaveTicket = 0 ;
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
	m_bRoomInfoDirty = false ;
	m_TimeSaveTicket = 0 ;
	m_nInformSerial = 0 ;
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
	m_nLittleBlind = pRoomConfig->nBigBlind * 0.5f ;
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

	// read players ;
	stMsgReadTaxasRoomPlayers msgReader ;
	msgReader.nRoomID = nRoomID ;
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nRoomID,(char*)&msgReader,sizeof(msgReader)) ;
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
	 m_TimeSaveTicket += fTimeElpas;
	 if ( m_TimeSaveTicket > TIME_SAVE_ROOM_INFO )
	 {
		 m_TimeSaveTicket = 0 ;
		 saveUpdateRoomInfo();
	 }

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
	case eRoomState_TP_Dead:
		{
			pState = new CTaxasStateDead; 
		}
		break;
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
		if ( pPeer && pPeer->nSessionID )
		{
			SendMsgToPlayer(pPeer->nSessionID,pMsg,nLen) ;
		}
		else
		{
#ifdef _DEBUG
			if ( pPeer == nullptr )
				CLogMgr::SharedLogMgr()->ErrorLog("why have null peer in m_vAllPeers") ;
#endif // DEBUG
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

	stTaxasInRoomPeerDataExten* pData = GetInRoomPlayerDataBySessionID(nPlayerSessionID) ;
	switch (prealMsg->usMsgType )
	{
	case MSG_TP_MODIFY_ROOM_NAME:
		{
			stMsgModifyTaxasRoomNameRet msgBack ;
			if ( isRoomAlive() == false )
			{
				msgBack.nRet = 2 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			stMsgModifyTaxasRoomName* pRet = (stMsgModifyTaxasRoomName*)prealMsg ;
			if ( !pData )
			{
				msgBack.nRet = 3 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}
	
			if ( m_nRoomOwnerUID != pData->nUserUID )
			{
				msgBack.nRet = 1 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			msgBack.nRet = 0 ;
			pRet->vNewRoomName[MAX_LEN_ROOM_NAME-1] = 0 ;
			setRoomName(pRet->vNewRoomName);
			SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
			m_bRoomInfoDirty = true ;
			return true;
		}
		break;
	case MSG_TP_MODIFY_ROOM_DESC:
		{
			stMsgModifyTaxasRoomDescRet msgBack ;
			if ( isRoomAlive() == false )
			{
				msgBack.nRet = 2 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			stMsgModifyTaxasRoomDesc* pRet = (stMsgModifyTaxasRoomDesc*)prealMsg ;
			if ( !pData )
			{
				msgBack.nRet = 3 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			if ( m_nRoomOwnerUID != pData->nUserUID )
			{
				msgBack.nRet = 1 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			if ( pRet->nLen >= MAX_LEN_ROOM_DESC )
			{
				msgBack.nRet = 4 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}
			msgBack.nRet = 0 ;
			char* pBuffer = new char[pRet->nLen + 1 ] ;
			memset(pBuffer,0,pRet->nLen + 1) ;
			memcpy(pBuffer,((char*)&pRet->nLen) + sizeof(pRet->nLen),pRet->nLen);
			setRoomDesc(pBuffer);
			SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
			delete[] pBuffer ;
			m_bRoomInfoDirty = true ;
			return true;
		}
		break;
	case MSG_TP_MODIFY_ROOM_INFORM:
		{
			stMsgModifyTaxasInformRet msgBack ;
			if ( isRoomAlive() == false )
			{
				msgBack.nRet = 2 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			stMsgModifyTaxasInform* pRet = (stMsgModifyTaxasInform*)prealMsg ;
			if ( !pData )
			{
				msgBack.nRet = 3 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			if ( m_nRoomOwnerUID != pData->nUserUID )
			{
				msgBack.nRet = 1 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			if ( pRet->nLen >= MAX_LEN_ROOM_DESC )
			{
				msgBack.nRet = 4 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}
			msgBack.nRet = 0 ;
			char* pBuffer = new char[pRet->nLen + 1 ] ;
			memset(pBuffer,0,pRet->nLen + 1) ;
			memcpy(pBuffer,((char*)&pRet->nLen) + sizeof(pRet->nLen),pRet->nLen);
			setRoomInform(pBuffer);
			SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
			delete[] pBuffer ;

			if ( m_strRoomInForm.empty() == false )
			{
				stMsgRemindTaxasRoomNewInform msgRemind ;
				SendRoomMsg(&msgRemind,sizeof(msgRemind)) ;
			}
			m_bRoomInfoDirty = true ;
		}
		break;
	case MSG_TP_CACULATE_ROOM_PROFILE:
		{
			stMsgCaculateTaxasRoomProfitRet msgBack ;
			msgBack.bDiamond = false ;
			msgBack.nProfitMoney = 0 ;
			if ( isRoomAlive() == false )
			{
				msgBack.nRet = 2 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			stMsgModifyTaxasRoomName* pRet = (stMsgModifyTaxasRoomName*)prealMsg ;
			if ( !pData )
			{
				msgBack.nRet = 3 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			if ( m_nRoomOwnerUID != pData->nUserUID )
			{
				msgBack.nRet = 1 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			if ( m_nRoomProfit < 10000 )
			{
				msgBack.nRet = 4 ;
				SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;
				return true;
			}

			msgBack.nRet = 0 ;
			msgBack.nProfitMoney = m_nRoomProfit - m_nRoomProfit % 10000;
			m_nRoomProfit = m_nRoomProfit % 10000 ;

			stMsgCrossServerRequest msgAddCoinReq ;
			msgAddCoinReq.cSysIdentifer = ID_MSG_PORT_DATA ;
			msgAddCoinReq.nReqOrigID = GetRoomID() ;
			msgAddCoinReq.nTargetID = pData->nUserUID ;
			msgAddCoinReq.vArg[0] = true ;
			msgAddCoinReq.vArg[1] = msgBack.nProfitMoney ;
			msgAddCoinReq.nRequestType = eCrossSvrReq_AddMoney ;
			msgAddCoinReq.nRequestSubType = eCrossSvrReqSub_TaxasRoomProfit ;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(msgAddCoinReq.nTargetID,(char*)&msgAddCoinReq,sizeof(msgAddCoinReq)) ;

			SendMsgToPlayer(nPlayerSessionID,&msgBack,sizeof(msgBack)) ;

			m_bRoomInfoDirty = true ;
			return true;
		}
		break;
	case MSG_TP_REQUEST_ROOM_INFORM:
		{
			stMsgRequestTaxasRoomInformRet msg ;
			if ( !pData )
			{
				msg.nLen = 0 ;
				SendMsgToPlayer(nPlayerSessionID,&msg,sizeof(msg)) ;
				return true ;
			}
			pData->m_nReadedInformSerial = m_nInformSerial ; 

			msg.nLen = strlen(m_strRoomInForm.c_str() );
			if ( msg.nLen == 0 )
			{
				SendMsgToPlayer(nPlayerSessionID,&msg,sizeof(msg)) ;
				return true ;
			}

			uint16_t nLen = sizeof(msg) + msg.nLen ;
			char* pBuffer = new char[nLen];
			memcpy(pBuffer,&msg,sizeof(msg));
			memcpy(pBuffer + sizeof(msg),m_strRoomInForm.c_str(),msg.nLen);
			SendMsgToPlayer(nPlayerSessionID,(stMsg*)pBuffer,nLen) ;
			delete[] pBuffer ;
		}
		break;
	case MSG_READ_TAXAS_ROOM_PLAYERS:
		{
			stMsgReadTaxasRoomPlayersRet* pRet = (stMsgReadTaxasRoomPlayersRet*)prealMsg ;
			stTaxasInRoomPeerDataExten* pData = new stTaxasInRoomPeerDataExten ;
			memset(pData,0,sizeof(stTaxasInRoomPeerDataExten));
			pData->m_nReadedInformSerial = pRet->m_nReadedInformSerial;
			pData->nFinalLeftInThisRoom = pRet->nFinalLeftInThisRoom ;
			pData->nPlayeTimesInThisRoom = pRet->nPlayeTimesInThisRoom ;
			pData->nTotalBuyInThisRoom = pRet->nTotalBuyInThisRoom ;
			pData->nWinTimesInThisRoom = pRet->nWinTimesInThisRoom ;
			pData->nUserUID = pRet->nPlayerUID ;
			m_vAllPeers.push_back(pData);
			CLogMgr::SharedLogMgr()->PrintLog("read taxas player uid = %d",pData->nUserUID) ;
		}
		break;
	case MSG_REQUEST_ROOM_DETAIL:
		{
			stMsgRequestRoomDetailRet msgRet ;
			msgRet.detailInfo.nCreatOwnerUID = getOwnerUID();
			msgRet.detailInfo.nCurrentCount = GetPlayerCntWithState(eRoomPeer_SitDown);
			msgRet.detailInfo.nRoomID = GetRoomID();
			msgRet.detailInfo.nSmiallBlind = getLittleBlind();
			msgRet.detailInfo.nSeatCnt = getSeatCnt();
			sprintf_s(msgRet.detailInfo.vRoomName,sizeof(msgRet.detailInfo.vRoomName),"%s",getRoomName());
			SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
		}
		break;
	case MSG_REQUEST_MY_OWN_ROOM_DETAIL:
		{
			stMsgRequestMyOwnRoomDetailRet msgRet ;
			msgRet.nCanWithdrawProfit = m_nRoomProfit ;
			msgRet.nConfigID = m_stRoomConfig.nConfigID ;
			msgRet.nDeadTime = m_nDeadTime ;
			msgRet.nFollows = 2 ;
			msgRet.nRoomID = GetRoomID() ;
			msgRet.nTotalProfit = 1000 ;
			sprintf_s(msgRet.vRoomName,sizeof(msgRet.vRoomName),"%s",getRoomName());
			SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CTaxasRoom::AddPlayer( stTaxasInRoomPeerDataExten& nPeerData )
{
	stTaxasInRoomPeerDataExten* pData = new stTaxasInRoomPeerDataExten ;
	memcpy(pData,&nPeerData,sizeof(stTaxasInRoomPeerDataExten));
	m_vAllPeers.push_back(pData);
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
	if ( pData == NULL /*|| pData->IsHaveState(eRoomPeer_WithdrawingCoin)*/ )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("nsession id = %d not in room cannot sit down or just withdrawing money ",nSessionID) ; 
		return ;
	}

	stTaxasPeerData& refSeatData = m_vSitDownPlayers[nSeatIdx];

	memset(&refSeatData,0,sizeof(refSeatData));
	memcpy(&refSeatData,pData,sizeof(stTaxasInRoomPeerData));
	refSeatData.nSeatIdx = nSeatIdx ;
	refSeatData.pHistoryData = pData ;
	refSeatData.eCurAct = eRoomPeerAction_None;
	refSeatData.nStateFlag = (eRoomPeer_WithdrawingCoin | eRoomPeer_SitDown );

	// arg ;
	stMsgCrossServerRequest msgCrossReq ;
	msgCrossReq.cSysIdentifer = ID_MSG_PORT_DATA ;
	msgCrossReq.nRequestType = eCrossSvrReq_DeductionMoney;
	msgCrossReq.nRequestSubType = eCrossSvrReqSub_TaxasSitDown ;
	msgCrossReq.nTargetID = pData->nUserUID ;
	msgCrossReq.nReqOrigID = GetRoomID() ;
	msgCrossReq.vArg[0] = true ;
	msgCrossReq.vArg[1] = nTakeInMoney ;
	msgCrossReq.vArg[2] = m_stRoomConfig.nMiniTakeInCoin ;
	Json::Value jsonArg ;
	jsonArg["seatIdx"] = nSeatIdx ;
	CON_REQ_MSG_JSON(msgCrossReq,jsonArg,pBuffer);
	CTaxasServerApp::SharedGameServerApp()->sendMsg(pData->nSessionID,pBuffer.getBufferPtr(),pBuffer.getContentSize()) ;
	CLogMgr::SharedLogMgr()->PrintLog("takin coin not enough request from data svr uid = %d" ,pData->nUserUID);


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

	if ( m_vSitDownPlayers[nSeatIdx].IsHaveState(eRoomPeer_WithdrawingCoin) )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("you are withdraw coin ,why you standup uid = %d",pData->nUserUID );
	}

	if ( m_vSitDownPlayers[nSeatIdx].IsHaveState(eRoomPeer_WithdrawingCoin) == false )
	{
		// tell data svr 
		syncPlayerDataToDataSvr(m_vSitDownPlayers[nSeatIdx]);
	}
	else
	{
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d standup while withdrawing money");
	}

	// write game result log 
	writePlayerResultLogToJson(m_vSitDownPlayers[nSeatIdx]);
	// save player room data 
	if ( pData->nPlayeTimesInThisRoom == m_vSitDownPlayers[nSeatIdx].nPlayTimes && pData->nPlayeTimesInThisRoom != 0 )
	{
		stMsgSaveTaxasRoomPlayer msgSave ;
		msgSave.isUpdate = false ;
		msgSave.m_nReadedInformSerial = pData->m_nReadedInformSerial ;
		msgSave.nFinalLeftInThisRoom = pData->nFinalLeftInThisRoom ;
		msgSave.nPlayerUID = pData->nUserUID ;
		msgSave.nPlayeTimesInThisRoom = pData->nPlayeTimesInThisRoom ;
		msgSave.nRoomID = GetRoomID() ;
		msgSave.nTotalBuyInThisRoom = pData->nTotalBuyInThisRoom ;
		msgSave.nWinTimesInThisRoom = pData->nWinTimesInThisRoom ;
		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msgSave,sizeof(msgSave)) ;
	}
	else
	{
		pData->bDataDirty = m_vSitDownPlayers[nSeatIdx].nPlayTimes != 0 ;
	}
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
		//msgLeave.nTakeInMoney = GetInRoomPlayerDataBySessionID(nPlayerSession)->nCoinInRoom ;
		//msgLeave.bIsDiamond = false ;
		CTaxasServerApp::SharedGameServerApp()->sendMsg(nPlayerSession,(char*)&msgLeave,sizeof(msgLeave)) ;
	}

	// remove from vec ;
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin() ;
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		stTaxasInRoomPeerDataExten* pData = *iter ;
		if ( (*iter)->nSessionID == nPlayerSession )
		{
			// save update 
			if ( pData->bDataDirty  )
			{
				stMsgSaveTaxasRoomPlayer msgSave ;
				msgSave.isUpdate = true ;
				msgSave.m_nReadedInformSerial = pData->m_nReadedInformSerial ;
				msgSave.nFinalLeftInThisRoom = pData->nFinalLeftInThisRoom ;
				msgSave.nPlayerUID = pData->nUserUID ;
				msgSave.nPlayeTimesInThisRoom = pData->nPlayeTimesInThisRoom ;
				msgSave.nRoomID = GetRoomID() ;
				msgSave.nTotalBuyInThisRoom = pData->nTotalBuyInThisRoom ;
				msgSave.nWinTimesInThisRoom = pData->nWinTimesInThisRoom ;
				CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msgSave,sizeof(msgSave)) ;
				CLogMgr::SharedLogMgr()->PrintLog("save update taxas player data uid = %d",pData->nUserUID);
			}
			pData->bDataDirty = false ;

			if ( (*iter)->nPlayeTimesInThisRoom > 0 )
			{
				(*iter)->nSessionID = 0 ;  // mark for not in this room , but have foot print for record
			}
			else
			{
				delete (*iter) ;
				(*iter) = NULL ;
				m_vAllPeers.erase(iter) ;
			}
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
				nValue = pData.nTakeInMoney ; // when all in must tell what value have allIned 
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,nValue);
			}
			pData.eCurAct = act ;
			pData.BetCoin(m_nMostBetCoinThisRound - pData.nBetCoinThisRound ) ;
		}
		break;
	case eRoomPeerAction_Add:
		{
			if ( pData.nTakeInMoney < nValue )
			{
				nValue = pData.nTakeInMoney ; // when all in must tell what value have allIned
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,nValue);
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

stTaxasPeerData* CTaxasRoom::GetSitDownPlayerDataByUID(uint32_t nUserUID)
{
	for each (stTaxasPeerData& pPlayer in m_vSitDownPlayers )
	{
		if ( pPlayer.IsInvalid() )
		{
			continue;
		}

		if ( pPlayer.nUserUID == nUserUID )
		{
			return &pPlayer ;
		}
	}
	return nullptr ;
}

// attribute and life
void CTaxasRoom::onCreateByPlayer(uint32_t nUserUID , uint16_t nRentDays )
{
	setOwnerUID(nUserUID);
	m_nCreateTime = time(nullptr);
	m_nDeadTime = m_nCreateTime + TIME_SECONDS_PER_DAY*nRentDays ;
}

void CTaxasRoom::setOwnerUID(uint32_t nCreatorUID )
{
	m_nRoomOwnerUID = nCreatorUID ;
}

void CTaxasRoom::addLiftTime(uint32_t nDays )
{
	m_nDeadTime += TIME_SECONDS_PER_DAY*nDays ;
}

void CTaxasRoom::setDeadTime(uint32_t nDeadTime)
{
	m_nDeadTime = nDeadTime ;
}

void CTaxasRoom::setAvataID(uint32_t nAvaID )
{
	m_nAvataID = nAvaID ;
}

void CTaxasRoom::setRoomName(const char* pRoomName)
{
	if ( pRoomName == nullptr || strlen(pRoomName) >= MAX_LEN_ROOM_NAME )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long proom name");
		return ;
	}

	memset(m_vRoomName,0,sizeof(m_vRoomName)) ;
	memcpy_s(m_vRoomName,MAX_LEN_ROOM_NAME,pRoomName,strlen(pRoomName));
}

void CTaxasRoom::setRoomDesc(const char* pRoomDesc )
{
	if ( pRoomDesc == nullptr || strlen(pRoomDesc) >= MAX_LEN_ROOM_DESC )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long proom desc ");
		return ;
	}

	m_strRoomDesc = pRoomDesc ;
}

void CTaxasRoom::setRoomInform(const char* pRoomInform )
{
	if ( pRoomInform == nullptr || strlen(pRoomInform) >= MAX_LEN_ROOM_DESC )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long proom Inform ");
		return ;
	}
	m_strRoomInForm = pRoomInform ;
}

bool CTaxasRoom::isRoomAlive()
{
	if ( m_nDeadTime == 0 )
	{
		return true ;
	}

	return time(NULL) > m_nDeadTime ;
}

void CTaxasRoom::setProfit(uint64_t nProfit )
{
	m_nRoomProfit = nProfit ;
}

void CTaxasRoom::setCreateTime(uint32_t nTime)
{
	m_nCreateTime = nTime ;
}

void CTaxasRoom::setInformSieral(uint32_t nSieaial)
{
	m_nInformSerial = nSieaial ;
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
	m_arrPlayers.clear();
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
		++pData.nPlayTimes ;
		++pData.pHistoryData->nPlayeTimesInThisRoom ;
		pData.nWinCoinThisGame = 0 ;
		if (pData.IsHaveState(eRoomPeer_WaitNextGame) )
		{
			pData.nStateFlag = eRoomPeer_CanAct ;

			// shou tai fei 
			m_nRoomProfit += m_stRoomConfig.nDeskFee ;
			pData.nTakeInMoney -= m_stRoomConfig.nDeskFee ;
			pData.pHistoryData->nFinalLeftInThisRoom -= m_stRoomConfig.nDeskFee ;
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

	m_bRoomInfoDirty = true ;
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
		if ( pData.nWinCoinThisGame > pData.nAllBetCoin )
		{
			++pData.nWinTimes ;
			++pData.pHistoryData->nWinTimesInThisRoom;
			uint64_t nRealWin = pData.nWinCoinThisGame - pData.nAllBetCoin ;
			pData.nSingleWinMost = pData.nSingleWinMost > nRealWin ? pData.nSingleWinMost : nRealWin ;
		}

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

		if (pData.nTakeInMoney < m_nLittleBlind * 4 + m_stRoomConfig.nDeskFee )
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

	// save serve log 
	writeGameResultLog();
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

void CTaxasRoom::debugPlayerHistory()
{
	CLogMgr::SharedLogMgr()->PrintLog("debug players history: ");
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin() ;
	stTaxasInRoomPeerDataExten* pPlayer = nullptr ;
	stTaxasPeerData* pSitDownPlayer = nullptr ;
	for ( ; iter != m_vAllPeers.end(); ++iter )
	{
		pPlayer = *iter ;
		uint64_t nRealLeft = pPlayer->nFinalLeftInThisRoom ;
		uint32_t nPlayTimes = pPlayer->nPlayeTimesInThisRoom ;
		uint32_t nWinTimes = pPlayer->nWinTimesInThisRoom ;
		int64_t nOffset = nRealLeft - pPlayer->nTotalBuyInThisRoom ;
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d , offset = %I64d totoalbuyin = %I64d, left = %I64d , playTimes = %d , winTimes = %d",pPlayer->nUserUID,nOffset,pPlayer->nTotalBuyInThisRoom,nRealLeft,nPlayTimes,nWinTimes);
	}
}

void CTaxasRoom::saveUpdateRoomInfo()
{
	if ( m_bRoomInfoDirty == false )
	{
		return ;
	}
	m_bRoomInfoDirty = false ;

	stMsgSaveUpdateTaxasRoomInfo msgSave ;
	msgSave.nAvataID = m_nAvataID ;
	msgSave.nDeadTime = m_nDeadTime ;
	msgSave.nInformSerial = m_nInformSerial ;
	msgSave.nRoomID = nRoomID ;
	msgSave.nRoomProfit = m_nRoomProfit ;
	memset(msgSave.vRoomDesc,0,sizeof(msgSave.vRoomDesc));
	memset(msgSave.vRoomName,0,sizeof(msgSave.vRoomName));
	//sprintf(msgSave.vRoomDesc,"%s",m_strRoomDesc.c_str());
	sprintf(msgSave.vRoomName,"%s",m_vRoomName );
	msgSave.nInformLen = strlen(m_strRoomInForm.c_str());
	if ( msgSave.nInformLen == 0 )
	{
		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msgSave,sizeof(msgSave)) ;
		return ;
	}

	CAutoBuffer autoBuffer(sizeof(msgSave) + msgSave.nInformLen);
	autoBuffer.addContent((char*)&msgSave,sizeof(msgSave)) ;
	autoBuffer.addContent(m_strRoomInForm.c_str(),msgSave.nInformLen) ;
	CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),autoBuffer.getBufferPtr(),autoBuffer.getContentSize()) ;
}

void CTaxasRoom::removeTaxasPlayersHistory()
{

}

void CTaxasRoom::writeGameResultLog()
{
	CLogMgr::SharedLogMgr()->PrintLog("write game result dlg");
	stMsgSaveLog saveMsg ;
	saveMsg.nLogType = eLog_TaxasGameResult ;
	saveMsg.nTargetID = GetRoomID();
	saveMsg.vArg[0] = getOwnerUID();
	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD; ++nIdx )
	{
		saveMsg.vArg[nIdx+1] = m_vPublicCardNums[nIdx];
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		writePlayerResultLogToJson(m_vSitDownPlayers[nIdx]) ;
	}

	Json::StyledWriter write ;
	std::string str = write.write(m_arrPlayers);
	CAutoBuffer auBuffer (sizeof(saveMsg) + str.size());
	auBuffer.addContent((char*)&saveMsg,sizeof(saveMsg)) ;
	auBuffer.addContent(str.c_str(),str.size());
	CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
	CLogMgr::SharedLogMgr()->PrintLog("all player info json str = %s" , str.c_str());
}

void CTaxasRoom::writePlayerResultLogToJson(stTaxasPeerData& pWritePlayer)
{
	if ( pWritePlayer.IsInvalid() )
	{
		return ;
	}

	if ( pWritePlayer.IsHaveState(eRoomPeer_StayThisRound) == false )
	{
		return ;
	}

	Json::Value refPlayer ;
	refPlayer["uid"] = pWritePlayer.nUserUID ;
	refPlayer["idx"] = pWritePlayer.nSeatIdx;
	refPlayer["card0"] = pWritePlayer.vHoldCard[0];
	refPlayer["card1"] = pWritePlayer.vHoldCard[1];
	refPlayer["betCoin"] = (uint32_t)pWritePlayer.nAllBetCoin ;
	refPlayer["offset"] = int32_t(pWritePlayer.nWinCoinThisGame - pWritePlayer.nAllBetCoin) ;
	refPlayer["coin"] = (int32_t)pWritePlayer.nTakeInMoney ;
	refPlayer["state"] = pWritePlayer.nStateFlag;
	m_arrPlayers[pWritePlayer.nSeatIdx] = refPlayer ;
	CLogMgr::SharedLogMgr()->PrintLog("write player uid = %d result log to json",pWritePlayer.nUserUID);
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
		m_vSitDownPlayers[nPeerIdx].pHistoryData->nFinalLeftInThisRoom += pPool.nCoin ;
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
		uint8_t nPeerIdx = pPool.vInPoolPlayerIdx[nIdx] ;
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
		pData.pHistoryData->nFinalLeftInThisRoom += nCoinPerWinner ;
		pData.nWinCoinThisGame += nCoinPerWinner ;
		CLogMgr::SharedLogMgr()->PrintLog("player use uid = %d win coin = %I64d , from pool idx = %d, room id = %d",pData.nUserUID,nCoinPerWinner,pPool.nIdx,nRoomID) ;
	}
}

void CTaxasRoom::SendRoomInfoToPlayer(uint32_t nSessionID )
{
	// send base info 
	stMsgTaxasRoomInfoBase msgBaseInfo ;
	msgBaseInfo.eCurRoomState = m_eCurRoomState ;
	msgBaseInfo.nOwnerUID = m_nRoomOwnerUID ;
	msgBaseInfo.nBankerIdx = m_nBankerIdx ;
	msgBaseInfo.nAvataID = m_nAvataID ;
	memcpy(msgBaseInfo.vRoomName,m_vRoomName,MAX_LEN_ROOM_NAME);
	msgBaseInfo.nBigBlindIdx = m_nBigBlindIdx ;
	msgBaseInfo.nCurMainBetPool = m_nCurMainBetPool;
	msgBaseInfo.nCurWaitPlayerActionIdx = m_nCurWaitPlayerActionIdx ;
	msgBaseInfo.nLittleBlind = m_nLittleBlind ;
	msgBaseInfo.nLittleBlindIdx = m_nLittleBlindIdx ;
	msgBaseInfo.nMaxSeat = (uint8_t)m_stRoomConfig.nMaxSeat;
	msgBaseInfo.nMostBetCoinThisRound = m_nMostBetCoinThisRound ;
	msgBaseInfo.nRoomID = nRoomID ;
	msgBaseInfo.nMiniTakeIn = m_stRoomConfig.nMiniTakeInCoin ;
	msgBaseInfo.nMaxTakeIn = m_stRoomConfig.nMaxTakeInCoin ;
	msgBaseInfo.nDeskFee = m_stRoomConfig.nDeskFee ;
	msgBaseInfo.nChatRoomID = m_nChatRoomID;
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

	if ( m_strRoomInForm.empty() == false )
	{
		stTaxasInRoomPeerDataExten* pData = GetInRoomPlayerDataBySessionID(nSessionID) ;
		assert(pData&&"can not be null");
		if ( pData->m_nReadedInformSerial < m_nInformSerial )
		{
			stMsgRemindTaxasRoomNewInform msgRemind ;
			SendMsgToPlayer(nSessionID,&msgRemind,sizeof(msgRemind)) ;
			CLogMgr::SharedLogMgr()->PrintLog("send new inform reminder to player in room");
		}
	}
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

stTaxasInRoomPeerDataExten* CTaxasRoom::GetInRoomPlayerDataByUID( uint32_t nUID )
{
	VEC_IN_ROOM_PEERS::iterator iter = m_vAllPeers.begin();
	for ( ; iter != m_vAllPeers.end() ; ++iter )
	{
		if ( (*iter) && (*iter)->nUserUID == nUID  )
		{
			return (*iter) ;
		}
	}
	return NULL;
}

void CTaxasRoom::syncPlayerDataToDataSvr( stTaxasPeerData& pPlayerData )
{
	// if player requesting coin , do not sync data ;
	stMsgCrossServerRequest msgReq ;
	msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
	msgReq.nReqOrigID = GetRoomID();
	msgReq.nTargetID = pPlayerData.nUserUID ;
	msgReq.nRequestType = eCrossSvrReq_AddMoney ;
	msgReq.nRequestSubType = eCrossSvrReqSub_TaxasStandUp ;
	msgReq.vArg[0] = true ;
	msgReq.vArg[1] = pPlayerData.nTakeInMoney;
	CTaxasServerApp::SharedGameServerApp()->sendMsg(pPlayerData.nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
	/// and just after game result ;
	if ( pPlayerData.nPlayTimes > 0 )
	{
		stMsgSyncTaxasPlayerData msg ;
		msg.nUserUID = pPlayerData.nUserUID ;
		msg.nPlayTimes = pPlayerData.nPlayTimes ;
		msg.nWinTimes = pPlayerData.nWinTimes ;
		msg.nSingleWinMost = pPlayerData.nSingleWinMost ;
		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msg,sizeof(msg)) ;
	}
}

bool CTaxasRoom::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	return false ;
}

bool CTaxasRoom::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_DeductionMoney == pResult->nRequestType && eCrossSvrReqSub_TaxasSitDown == pResult->nRequestSubType )
	{
		stTaxasInRoomPeerDataExten* pPlayrInRoomData = GetInRoomPlayerDataByUID(pResult->nReqOrigID);
		int8_t nSeatIdx = (*vJsValue)["seatIdx"].asInt();
		uint64_t nMoney = pResult->vArg[1];
		if ( pResult->nRet )
		{
			CLogMgr::SharedLogMgr()->PrintLog("sit down get coin error , not enough uid = %d",pResult->nReqOrigID);
			// player still at seat when money arrived ;
			if ( nSeatIdx < m_stRoomConfig.nMaxSeat && m_vSitDownPlayers[nSeatIdx].IsInvalid() == false && m_vSitDownPlayers[nSeatIdx].nUserUID == pResult->nReqOrigID  )
			{
				OnPlayerStandUp(nSeatIdx);
			}

			if ( pPlayrInRoomData && pPlayrInRoomData->nSessionID )
			{
				// still in room inform sit ret ;
				stMsgWithdrawingMoneyRet msgRet ;
				msgRet.nRet = 1 ;
				SendMsgToPlayer(pPlayrInRoomData->nSessionID,&msgRet,sizeof(msgRet)) ;
			}
		}
		else
		{
			if ( nSeatIdx > m_stRoomConfig.nMaxSeat || m_vSitDownPlayers[nSeatIdx].IsInvalid() || m_vSitDownPlayers[nSeatIdx].nUserUID != pResult->nReqOrigID  )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("money arrived ,but you have gone standup = %d",pResult->nReqOrigID ) ;
				if ( pPlayrInRoomData && pPlayrInRoomData->nSessionID )
				{
					// still in room inform sit ret ;
					stMsgWithdrawingMoneyRet msgRet ;
					msgRet.nRet = 2 ;
					SendMsgToPlayer(pPlayrInRoomData->nSessionID,&msgRet,sizeof(msgRet)) ;
				}

				// give back coin 
				stMsgCrossServerRequest msgReq ;
				msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
				msgReq.nReqOrigID = GetRoomID();
				msgReq.nTargetID = pResult->nReqOrigID ;
				msgReq.nRequestType = eCrossSvrReq_AddMoney;
				msgReq.nRequestSubType = eCrossSvrReqSub_TaxasSitDownFailed;
				msgReq.vArg[0] = pResult->vArg[0];
				msgReq.vArg[1] = pResult->vArg[1];
				CTaxasServerApp::SharedGameServerApp()->sendMsg(msgReq.nTargetID,(char*)&msgReq,sizeof(msgReq)) ;
			}
			else
			{
				m_vSitDownPlayers[nSeatIdx].nStateFlag = eRoomPeer_WaitNextGame ;
				m_vSitDownPlayers[nSeatIdx].nTakeInMoney += nMoney;
				m_vSitDownPlayers[nSeatIdx].nTotalBuyInThisRoom += nMoney ;

				stMsgTaxasRoomUpdatePlayerState msgNewState ;
				msgNewState.nSeatIdx = nSeatIdx ;
				msgNewState.nStateFlag = eRoomPeer_WaitNextGame ;
				msgNewState.nTakeInCoin = m_vSitDownPlayers[nSeatIdx].nTakeInMoney ;

				SendRoomMsg(&msgNewState,sizeof(msgNewState));

				CLogMgr::SharedLogMgr()->PrintLog("withdraw coin ok uid = %d coin = %I64d",pResult->nReqOrigID,msgNewState.nTakeInCoin );
				// caculate player history in this room 
				if ( nMoney > pPlayrInRoomData->nFinalLeftInThisRoom )
				{
					uint64_t nNewExtBuyIn = nMoney - pPlayrInRoomData->nFinalLeftInThisRoom ;
					pPlayrInRoomData->nTotalBuyInThisRoom += nNewExtBuyIn;
					pPlayrInRoomData->nFinalLeftInThisRoom += nNewExtBuyIn; 
				}
			}
		}

		return true ;
	}
	return false ;
}
