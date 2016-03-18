#include "TaxasRoom.h"
#include "LogManager.h"
#include "TaxasRoomState.h"
#include "TaxasServerApp.h"
#include "TaxasPokerPeerCard.h"
#include "ServerMessageDefine.h"
#include <json/json.h>
#include "TaxasServerApp.h"
#include "RoomManager.h"
#include "AutoBuffer.h"
#include "ServerStringTable.h"
#include "TaxasPlayer.h"
#include <algorithm>
#define TIME_SECONDS_PER_DAY (60*60*24)
#define TIME_SAVE_ROOM_INFO 60*30
CTaxasRoom::CTaxasRoom()
{
	
	m_nLittleBlind = 0 ;
	m_nMinTakeIn = 0 ;
	m_nMaxTakeIn = 0;
	// running members ;
	m_nBankerIdx = 0 ;
	m_nLittleBlindIdx = 0 ;
	m_nBigBlindIdx = 0;
	m_nCurWaitPlayerActionIdx = -1 ;
	m_nCurMainBetPool  = 0 ;
	uint64_t  m_nMostBetCoinThisRound = 0 ;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nPublicCardRound = 0 ;
	for ( auto& pool : m_vAllVicePools )
	{
		pool.Reset();
	}

	getPoker()->InitTaxasPoker() ;
}

CTaxasRoom::~CTaxasRoom()
{
 
}

uint8_t CTaxasRoom::getRoomType()
{
	return eRoom_TexasPoker ;
}

bool CTaxasRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue )
{
	stTaxasRoomConfig* pRoomConfig = (stTaxasRoomConfig*)pConfig ;
	m_nLittleBlind = pRoomConfig->nBigBlind * 0.5f ;
	m_nMinTakeIn = pRoomConfig->nMiniTakeInCoin ;
	m_nMaxTakeIn = pRoomConfig->nMaxTakeInCoin;
	ISitableRoom::init(pConfig,nRoomID,vJsValue) ;
	return true ;
}

void CTaxasRoom::serializationFromDB(stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	stTaxasRoomConfig* pRoomConfig = (stTaxasRoomConfig*)pConfig ;
	m_nLittleBlind = pRoomConfig->nBigBlind * 0.5f ;
	m_nMinTakeIn = pRoomConfig->nMiniTakeInCoin ;
	m_nMaxTakeIn = pRoomConfig->nMaxTakeInCoin;
	ISitableRoom::serializationFromDB(pConfig,nRoomID,vJsValue) ;
}

void CTaxasRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	ISitableRoom::willSerializtionToDB(vOutJsValue) ;
}

void CTaxasRoom::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	ISitableRoom::roomItemDetailVisitor(vOutJsValue) ;
	//vOutJsValue["bigBlind"] = getLittleBlind() * 2 ;
	//vOutJsValue["minTakeIn"] = getMinTakeIn() ;
	//vOutJsValue["maxTakeIn"] = getMaxTakeIn() ;
}

void CTaxasRoom::prepareState()
{
	ISitableRoom::prepareState();
	// create room state ;
	IRoomState* vState[] = {
		new CTaxasStateStartGame(),new CTaxasStatePlayerBet(),new CTaxasStateOneRoundBetEndResult(),
		new CTaxasStatePublicCard(),new CTaxasStateGameResult()
	};
	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
}

void CTaxasRoom::sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID )
{
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

bool CTaxasRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( ISitableRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch (prealMsg->usMsgType )
	{
	case MSG_TP_REQUEST_ROOM_INFO:
		{
			CLogMgr::SharedLogMgr()->SystemLog("send room info to player session id = %d",nPlayerSessionID );
			sendRoomInfoToPlayer(nPlayerSessionID);
		}
		break;
	//case MSG_REQUEST_ROOM_DETAIL:
	//	{
	//		stMsgRequestRoomDetailRet msgRet ;
	//		msgRet.detailInfo.nCreatOwnerUID = getOwnerUID();
	//		msgRet.detailInfo.nCurrentCount = GetPlayerCntWithState(eRoomPeer_SitDown);
	//		msgRet.detailInfo.nRoomID = GetRoomID();
	//		msgRet.detailInfo.nSmiallBlind = getLittleBlind();
	//		msgRet.detailInfo.nSeatCnt = getSeatCnt();
	//		sprintf_s(msgRet.detailInfo.vRoomName,sizeof(msgRet.detailInfo.vRoomName),"%s",getRoomName());
	//		SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
	//	}
	//	break;
	//case MSG_REQUEST_MY_OWN_ROOM_DETAIL:
	//	{
	//		stMsgRequestMyOwnRoomDetailRet msgRet ;
	//		msgRet.nRet = 0 ;
	//		msgRet.nRoomType = eRoom_TexasPoker ;
	//		msgRet.nCanWithdrawProfit = m_nRoomProfit ;
	//		msgRet.nConfigID = m_stRoomConfig.nConfigID ;
	//		msgRet.nDeadTime = m_nDeadTime ;
	//		msgRet.nFollows = 2 ;
	//		msgRet.nRoomID = GetRoomID() ;
	//		msgRet.nTotalProfit = m_nTotalProfit ;
	//		sprintf_s(msgRet.vRoomName,sizeof(msgRet.vRoomName),"%s",getRoomName());
	//		SendMsgToPlayer(nPlayerSessionID,&msgRet,sizeof(msgRet)) ;
	//	}
	//	break;
	case MSG_TP_CHANGE_ROOM:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("change room function canncel");
			return true ;
			stMsgTaxasPlayerLeave leave ;
			onMessage(&leave,ID_MSG_PORT_CLIENT,nPlayerSessionID);
			
			return true ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

ISitableRoomPlayer* CTaxasRoom::doCreateSitableRoomPlayer()
{
	auto p = new CTaxasPlayer ;
	return p ;
}

uint32_t CTaxasRoom::coinNeededToSitDown()
{
	if ( m_nLittleBlind*20 > m_nMinTakeIn )
	{
		return m_nLittleBlind*20 ;
	}

	return m_nMinTakeIn ;
}

bool sortPlayerByCard(ISitableRoomPlayer* pLeft , ISitableRoomPlayer* pRight )
{
	CTaxasPlayer* pNLeft = (CTaxasPlayer*)pLeft ;
	CTaxasPlayer* pNRight = (CTaxasPlayer*)pRight ;
	if ( pNLeft->getPeerCard()->PK(pNRight->getPeerCard()) == -1 )
	{
		return true ;
	}
	return false ;
}

void CTaxasRoom::prepareCards()
{
	// parepare cards for all player ;
	for ( uint8_t nPIdx = 0 ; nPIdx < TAXAS_PUBLIC_CARD ; ++nPIdx )
	{
		m_vPublicCardNums[nPIdx] = getPoker()->GetCardWithCompositeNum();
	}

	uint8_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CTaxasPlayer* pRoomPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = 2 ;
			uint8_t nCardIdx = 0 ;
			while ( nCardIdx < nCardCount )
			{
				pRoomPlayer->addPeerCard(nCardIdx,getPoker()->GetCardWithCompositeNum());
				++nCardIdx ;
			}

			// add public cards
			pRoomPlayer->addPublicCard(m_vPublicCardNums) ;
			m_vSortByPeerCardsAsc.push_back(pRoomPlayer) ;
		}
	}

	std::sort(m_vSortByPeerCardsAsc.begin(),m_vSortByPeerCardsAsc.end(),sortPlayerByCard);

	doProcessNewPlayerHalo();
}

void CTaxasRoom::onPlayerWillStandUp(ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer->isHaveState(eRoomPeer_WaitCaculate) )
	{
		ISitableRoom::onPlayerWillStandUp(pPlayer) ;
		return ;
	}

	if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) )
	{
		if ( getCurRoomState()->getStateID() != eRoomState_TP_GameResult )
		{
			CTaxasPlayer* p = (CTaxasPlayer*)pPlayer ;
			CLogMgr::SharedLogMgr()->ErrorLog("will stand up update offset uid = %d",pPlayer->getUserUID());
			updatePlayerOffset(pPlayer->getUserUID(),p->getCoinOffsetThisGame());
		}
		else
		{
			CLogMgr::SharedLogMgr()->SystemLog("if here update player uid = %d offeset will occure a bug ",pPlayer->getUserUID() ) ;
		}
	}
	playerDoStandUp(pPlayer);
}

uint32_t CTaxasRoom::getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)
{
	return 0 ;
}

uint8_t CTaxasRoom::OnPlayerAction( uint8_t nSeatIdx ,eRoomPeerAction act , uint64_t& nValue )
{
	if ( nSeatIdx >= getSeatCount() || getPlayerByIdx(nSeatIdx) == nullptr )
	{
		return 2 ;
	}

	if ( nSeatIdx != m_nCurWaitPlayerActionIdx && act != eRoomPeerAction_GiveUp )
	{
		return 1 ;
	}

	if ( getPlayerByIdx(nSeatIdx)->isHaveState(eRoomPeer_CanAct) == false )
	{
		return 3 ;
	}

	CTaxasPlayer* pData = (CTaxasPlayer*)getPlayerByIdx(nSeatIdx) ;
	switch ( act )
	{
	case eRoomPeerAction_GiveUp:
		{
			m_nCurMainBetPool += pData->getBetCoinThisRound() ;
			pData->setCurActType(eRoomPeerAction_GiveUp);
			pData->setState(eRoomPeer_GiveUp);
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
			if ( pData->getCoin() + pData->getBetCoinThisRound() <= m_nMostBetCoinThisRound )
			{
				nValue = pData->getCoin() ; // when all in must tell what value have allIned 
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,nValue);
			}
			pData->setCurActType(act);
			pData->betCoin(m_nMostBetCoinThisRound - pData->getBetCoinThisRound()) ;
		}
		break;
	case eRoomPeerAction_Add:
		{
			if ( pData->getCoin() <= nValue )
			{
				nValue = pData->getCoin() ; // when all in must tell what value have allIned
				return OnPlayerAction(nSeatIdx,eRoomPeerAction_AllIn,nValue);
			}

			if ( pData->getBetCoinThisRound() + nValue < m_nMostBetCoinThisRound + m_nLittleBlind * 2  )
			{
				return 6 ;
			}

			if ( ((pData->getBetCoinThisRound() + nValue) - m_nMostBetCoinThisRound ) % (m_nLittleBlind * 2) != 0  )
			{
				return 7 ;
			}

			pData->setCurActType(act);
			pData->betCoin(nValue) ;
			m_nMostBetCoinThisRound = pData->getBetCoinThisRound() ;
		}
		break;
	case eRoomPeerAction_AllIn:
		{
			pData->setCurActType(act);
			pData->setState(eRoomPeer_AllIn);
			nValue = pData->getCoin() ;
			pData->betCoin(nValue) ;
			if ( pData->getBetCoinThisRound() == 0 )
			{
				pData->setBetCoinThisRound(1);// avoid 0 all In bug ;
				CLogMgr::SharedLogMgr()->ErrorLog("room id = %d , 0 coin all in player idx = %d, uid = %d",getRoomID(),nSeatIdx,pData->getUserUID()) ;
			}

			if ( pData->getBetCoinThisRound() > m_nMostBetCoinThisRound )
			{
				m_nMostBetCoinThisRound = pData->getBetCoinThisRound() ;
			}
		}
		break;
	case eRoomPeerAction_Pass:
		{
			if ( m_nMostBetCoinThisRound != pData->getBetCoinThisRound() )
			{
				return 5 ;
			}

			pData->setCurActType(eRoomPeerAction_Pass);
		}
		break;
	default:
		return 4 ;
	}

	stMsgTaxasRoomAct msgOtherAct ;
	msgOtherAct.nPlayerAct = act ;
	msgOtherAct.nPlayerIdx = nSeatIdx ;
	msgOtherAct.nValue = nValue ;
	sendRoomMsg(&msgOtherAct,sizeof(msgOtherAct)) ;
	CLogMgr::SharedLogMgr()->PrintLog("player do act") ;

	if ( pData->isDelayStandUp() && act == eRoomPeerAction_GiveUp )
	{
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d have delay standup , give up act , right standup and update offset ",pData->getUserUID());
		updatePlayerOffset(pData->getUserUID(),pData->getCoinOffsetThisGame());
		playerDoStandUp(pData);	
	}

	return 0 ;
}

// logic function 
void CTaxasRoom::onGameWillBegin()
{
	m_arrPlayers.clear();
	// parepare all players ;
	getPoker()->RestAllPoker();
	ISitableRoom::onGameWillBegin();
	// prepare running data 
	m_nCurWaitPlayerActionIdx = -1;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	m_nPublicCardRound = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}
}

void CTaxasRoom::onGameDidEnd()
{
	// parepare all players ;
	ISitableRoom::onGameDidEnd();

	// prepare running data 
	m_nCurWaitPlayerActionIdx = -1;
	m_nCurMainBetPool = 0;
	m_nMostBetCoinThisRound = 0;
	memset(m_vPublicCardNums,0,sizeof(m_vPublicCardNums)) ;
	m_nPublicCardRound = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vAllVicePools[nIdx].nIdx = nIdx ;
		m_vAllVicePools[nIdx].Reset();
	}
}

void CTaxasRoom::startGame()
{
	// init running data 
	m_nBankerIdx = GetFirstInvalidIdxWithState(m_nBankerIdx + 1 , eRoomPeer_CanAct) ;
	m_nLittleBlindIdx = GetFirstInvalidIdxWithState(m_nBankerIdx + 1 , eRoomPeer_CanAct) ;
	m_nBigBlindIdx = GetFirstInvalidIdxWithState(m_nLittleBlindIdx + 1 , eRoomPeer_CanAct) ;

	// bet coin this 
	auto pTaxPlayer = (CTaxasPlayer*)getPlayerByIdx(m_nLittleBlindIdx);
	pTaxPlayer->betCoin( m_nLittleBlind ) ;

	pTaxPlayer = (CTaxasPlayer*)getPlayerByIdx(m_nBigBlindIdx);
	pTaxPlayer->betCoin( m_nLittleBlind * 2 ) ;
	m_nMostBetCoinThisRound = m_nLittleBlind * 2 ;

	stMsgTaxasRoomStartRound msgStart ;
	msgStart.nBankerIdx = m_nBankerIdx ;
	msgStart.nBigBlindIdx = m_nBigBlindIdx ;
	msgStart.nLittleBlindIdx = m_nLittleBlindIdx ;
	sendRoomMsg(&msgStart,sizeof(msgStart));

	// send card msg ;
	stMsgTaxasRoomPrivateCard msgPrivate ;
	msgPrivate.nPlayerCnt = getPlayerCntWithState(eRoomPeer_CanAct) ;
	uint16_t nBuferLen = sizeof(msgPrivate) + sizeof(stTaxasHoldCardItems) * msgPrivate.nPlayerCnt;
	char* pBuffer = new char[nBuferLen] ;
	uint16_t nOffset = 0 ;
	memcpy(pBuffer,&msgPrivate,sizeof(msgPrivate));
	nOffset += sizeof(msgPrivate);

	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx)
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || ( pPlayer->isHaveState(eRoomPeer_CanAct) == false ) )
		{
			continue; 
		}

		stTaxasHoldCardItems privateCards ;
		privateCards.cPlayerIdx = nIdx ;
		privateCards.vCards[0] = pPlayer->getPeerCardByIdx(0);
		privateCards.vCards[1] = pPlayer->getPeerCardByIdx(1);
		memcpy(pBuffer + nOffset , &privateCards,sizeof(privateCards) );
		nOffset += sizeof(privateCards);
	}

	if ( nOffset != nBuferLen )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("buffer error for private cards") ;
		return ;
	}
	sendRoomMsg((stMsg*)pBuffer,nBuferLen) ;
	delete[] pBuffer ;
	pBuffer = NULL ;
}

void CTaxasRoom::PreparePlayersForThisRoundBet()
{
	m_nMostBetCoinThisRound = 0 ;
	if ( m_nCurWaitPlayerActionIdx >= 0 )  // means not first round 
	{
		m_nCurWaitPlayerActionIdx = m_nLittleBlindIdx - 1 ; //  little blid begin act   ps: m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState( m_nCurWaitPlayerActionIdx + 1 ,eRoomPeer_CanAct) ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx)
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || ( pPlayer->isHaveState(eRoomPeer_StayThisRound) == false ) )
		{
			continue; 
		}

		pPlayer->setBetCoinThisRound(0);
		pPlayer->setCurActType(eRoomPeerAction_None) ;
	}
}

uint8_t CTaxasRoom::InformPlayerAct()
{
	if ( m_nCurWaitPlayerActionIdx < 0 ) // first round 
	{
		CLogMgr::SharedLogMgr()->PrintLog("fist round");
		m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState(m_nBigBlindIdx + 1 ,eRoomPeer_CanAct) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->PrintLog("second round");
		m_nCurWaitPlayerActionIdx = GetFirstInvalidIdxWithState( m_nCurWaitPlayerActionIdx + 1 ,eRoomPeer_CanAct) ;
	}

	stMsgTaxasRoomWaitPlayerAct msgWait ;
	msgWait.nActPlayerIdx = m_nCurWaitPlayerActionIdx ;
	sendRoomMsg(&msgWait,sizeof(msgWait));
	CLogMgr::SharedLogMgr()->PrintLog("room id = %d ,wait idx = %d act ",getRoomID(),m_nCurWaitPlayerActionIdx ) ;
	return m_nCurWaitPlayerActionIdx ;
}

void CTaxasRoom::OnPlayerActTimeOut()
{
	stMsgTaxasPlayerAct msg ;
	msg.nValue = 0 ;
	msg.nRoomID = getRoomID() ;
	CTaxasPlayer* pPlayer = (CTaxasPlayer*)getPlayerByIdx(m_nCurWaitPlayerActionIdx);
	if ( pPlayer == nullptr )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why cur wait player is null ");
		return ;
	}

	if ( m_nMostBetCoinThisRound == pPlayer->getBetCoinThisRound() )
	{
		msg.nPlayerAct = eRoomPeerAction_Pass ;
	}
	else
	{
		msg.nPlayerAct = eRoomPeerAction_GiveUp ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("wait player act time out auto act room id = %d",getRoomID()) ;
	onMessage(&msg,ID_MSG_PORT_CLIENT,pPlayer->getSessionID()) ;
}

bool CTaxasRoom::IsThisRoundBetOK()
{
	if ( getPlayerCntWithState(eRoomPeer_WaitCaculate) == 1 )
	{
		return true ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx)
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			continue; 
		}
		
		if ( (pPlayer->getCurActType() == eRoomPeerAction_None || pPlayer->getBetCoinThisRound() != m_nMostBetCoinThisRound ) )
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
		for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
		{
			auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
			if ( pPlayer == nullptr || (pPlayer->isHaveState(eRoomPeer_WaitCaculate) == false ) )
			{
				continue;
			}

			if ( pPlayer->getCurActType() == eRoomPeerAction_AllIn && pPlayer->getBetCoinThisRound() > 0 )
			{
				if ( pPlayer->getBetCoinThisRound() < nVicePool || nVicePool == 0 )
				{
					nVicePool = pPlayer->getBetCoinThisRound() ;
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
		for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
		{
			auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
			if ( pPlayer == nullptr || (pPlayer->isHaveState(eRoomPeer_WaitCaculate) == false ) )
			{
				continue;
			}

			if ( pPlayer->getBetCoinThisRound() > 0 )
			{
				if ( pPlayer->getBetCoinThisRound() < nVicePool )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("room id = %d , put vice pool coin not enough , why error error",getRoomID()) ;
				}
				pPool.nCoin += nVicePool ;
				pPlayer->setBetCoinThisRound(pPlayer->getBetCoinThisRound() - nVicePool ) ;
				pPool.vInPoolPlayerIdx.push_back(nIdx) ;
				CLogMgr::SharedLogMgr()->PrintLog("put player into pool player Idx = %d, UID = %d",nIdx,pPlayer->getUserUID() ) ;
			}
		}
		CLogMgr::SharedLogMgr()->SystemLog("pool idx = %d : coin = %I64d",pPool.nIdx,pPool.nCoin) ;
	}

	// build mian pool ;
	CLogMgr::SharedLogMgr()->PrintLog("build main pool: " ) ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
	{
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || (pPlayer->isHaveState(eRoomPeer_CanAct) == false ) )
		{
			continue;
		}

		if ( pPlayer->getBetCoinThisRound() > 0 )
		{
			m_nCurMainBetPool += pPlayer->getBetCoinThisRound() ;
			pPlayer->setBetCoinThisRound(0);
			CLogMgr::SharedLogMgr()->PrintLog("put player into Main pool player Idx = %d, UID = %d",nIdx,pPlayer->getUserUID()) ;
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
	sendRoomMsg(&msgResult,sizeof(msgResult)) ;
	return nProducedVicePoolCnt ;
}

// return dis card cnt ;
uint8_t CTaxasRoom::DistributePublicCard()
{
	CLogMgr::SharedLogMgr()->PrintLog("pulick card = %d",m_nPublicCardRound);
	stMsgTaxasRoomPublicCard msgPublicCard ;
	msgPublicCard.nCardSeri = m_nPublicCardRound++ ;
	// distr 3 
	if ( m_nPublicCardRound == 1 )
	{
		for ( uint8_t nIdx = 0 ; nIdx < 3 ; ++nIdx )
		{
			msgPublicCard.vCard[nIdx] = m_vPublicCardNums[nIdx] ;
		}
		// send msg to tell client ;
		sendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 3 ;
	}

	if ( m_nPublicCardRound == 2 )
	{
		msgPublicCard.vCard[0] = m_vPublicCardNums[3] ;
		sendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
		return 1 ;
	}

	if ( m_nPublicCardRound == 3 )
	{
		msgPublicCard.vCard[0] = m_vPublicCardNums[4] ;
		sendRoomMsg(&msgPublicCard,sizeof(msgPublicCard)) ;
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
		for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
		{
			auto pPlayer = getPlayerByIdx(nIdx);
			if ( nullptr == pPlayer )
			{
				continue;
			}

			if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
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
		CLogMgr::SharedLogMgr()->ErrorLog("why this game have no pool ? at least should have one room id = %d",getRoomID() ) ;
		didCaculateGameResult();
		return 0 ;
	}

	uint8_t nLastPoolIdx = GetFirstCanUseVicePool().nIdx ;
	bool bSendEndFlag = false ;
	for ( uint8_t nIdx = 0 ; nIdx < nLastPoolIdx; ++nIdx )
	{
		stVicePool& pool = m_vAllVicePools[nIdx] ;
		if ( pool.vWinnerIdxs.empty() )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("this pool have no winners , coin = %I64d, room = %d ",pool.nCoin,getRoomID() ) ;
			continue;
		}

		stMsgTaxasRoomGameResult msgResult ;
		msgResult.nCoinPerWinner = pool.nCoin / pool.vWinnerIdxs.size() ;
		msgResult.nPoolIdx = nIdx ;
		msgResult.nWinnerCnt = 0;
		msgResult.bIsLastOne = (nIdx + 1) >= nLastPoolIdx ;
		CLogMgr::SharedLogMgr()->PrintLog("game reuslt pool idx = %d  isLast one = %d",nIdx,msgResult.bIsLastOne ) ;
		VEC_INT8::iterator iter = pool.vWinnerIdxs.begin() ;
		for ( ; iter != pool.vWinnerIdxs.end() ; ++iter )
		{
			msgResult.vWinnerIdx[msgResult.nWinnerCnt++] = (*iter); 
		}
		sendRoomMsg(&msgResult,sizeof(msgResult)) ;
		if ( msgResult.bIsLastOne )
		{
			bSendEndFlag = true ;
		}
	}

	if ( bSendEndFlag == false )  // must have a end flag msg ;
	{
		stMsgTaxasRoomGameResult msgResult ;
		msgResult.nCoinPerWinner = 0;
		msgResult.nPoolIdx = nLastPoolIdx ;
		msgResult.nWinnerCnt = 0;
		msgResult.bIsLastOne = true ;
		sendRoomMsg(&msgResult,sizeof(msgResult)) ;
	}

	didCaculateGameResult();
	return GetFirstCanUseVicePool().nIdx ;
}

//uint64_t CTaxasRoom::GetAllBetCoinThisRound()
//{
//	uint64_t nCoinThis = 0 ;
//	for ( uint8_t nIdx = 0 ; nIdx < m_stRoomConfig.nMaxSeat ; ++nIdx)
//	{
//		if ( m_vSitDownPlayers[nIdx].IsInvalid() || ( m_vSitDownPlayers[nIdx].IsHaveState(eRoomPeer_StayThisRound) == false ) )
//		{
//			continue; 
//		}
//
//		stTaxasPeerData& pData = m_vSitDownPlayers[nIdx] ;
//		nCoinThis += pData.nBetCoinThisRound;
//	}
//	return nCoinThis ;
//}

bool CTaxasRoom::IsPublicDistributeFinish()
{
	return (m_nPublicCardRound >= 3 );
}

void CTaxasRoom::didCaculateGameResult()
{
	// save serve log 
	writeGameResultLog();
	
	// update coin offset ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount(); ++nIdx )
	{
		CTaxasPlayer* pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_StayThisRound) == false ) 
		{
			continue;
		}
		CLogMgr::SharedLogMgr()->ErrorLog("game end update offset");
		updatePlayerOffset(pPlayer->getUserUID(),pPlayer->getCoinOffsetThisGame());
	}

	debugRank();
	//// update best card ;
	//if ( IsPublicDistributeFinish() )
	//{
	//	CTaxasPokerPeerCard peerCur , peerBest;
	//	for ( stTaxasPeerData& peer : m_vSitDownPlayers )
	//	{
	//		if ( peer.IsInvalid() || peer.IsHaveState(eRoomPeer_StayThisRound) == false || peer.IsHaveState(eRoomPeer_GiveUp) )
	//		{
	//			continue;
	//		}

	//		peerCur.Reset();
	//		peerBest.Reset() ;
	//		for ( uint8_t& comN : m_vPublicCardNums )
	//		{
	//			peerCur.AddCardByCompsiteNum(comN);
	//		}
	//		peerCur.AddCardByCompsiteNum(peer.vHoldCard[0]) ;
	//		peerCur.AddCardByCompsiteNum(peer.vHoldCard[1]) ;

	//		if ( peer.vBestCards[0] == 0 )
	//		{
	//			peerCur.GetCardType() ;
	//			peerCur.GetFinalCard(peer.vBestCards);
	//			continue;
	//		}

	//		for ( uint8_t& c : peer.vBestCards )
	//		{
	//			peerBest.AddCardByCompsiteNum(c);
	//		}

	//		if ( peerCur.PK(&peerBest) == 1 )
	//		{
	//			peerCur.GetFinalCard(peer.vBestCards);
	//		}
	//	}
	//}

}

void CTaxasRoom::writeGameResultLog()
{
	CLogMgr::SharedLogMgr()->PrintLog("write game result dlg");
	stMsgSaveLog saveMsg ;
	saveMsg.nLogType = eLog_TaxasGameResult ;
	saveMsg.nTargetID = getRoomID();
	saveMsg.vArg[0] = getOwnerUID();
	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD; ++nIdx )
	{
		saveMsg.vArg[nIdx+1] = m_vPublicCardNums[nIdx];
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		writePlayerResultLogToJson((CTaxasPlayer*)getPlayerByIdx(nIdx)) ;
	}

	Json::StyledWriter write ;
	std::string str = write.write(m_arrPlayers);
	CAutoBuffer auBuffer (sizeof(saveMsg) + str.size());
	saveMsg.nJsonExtnerLen = str.size() ;
	auBuffer.addContent((char*)&saveMsg,sizeof(saveMsg)) ;
	auBuffer.addContent(str.c_str(),str.size());
	CTaxasServerApp::SharedGameServerApp()->sendMsg(getRoomID(),auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
	CLogMgr::SharedLogMgr()->PrintLog("all player info json str = %s" , str.c_str());
}

void CTaxasRoom::writePlayerResultLogToJson(CTaxasPlayer* pWritePlayer)
{
	if ( !pWritePlayer )
	{
		return ;
	}

	if ( pWritePlayer->isHaveState(eRoomPeer_StayThisRound) == false )
	{
		return ;
	}

	Json::Value refPlayer ;
	refPlayer["uid"] = pWritePlayer->getUserUID() ;
	refPlayer["idx"] = pWritePlayer->getIdx();
	refPlayer["card0"] = pWritePlayer->getPeerCardByIdx(0);
	refPlayer["card1"] = pWritePlayer->getPeerCardByIdx(1);
	refPlayer["betCoin"] = (uint32_t)pWritePlayer->getAllBetCoin();
	refPlayer["offset"] = int32_t(pWritePlayer->getCoinOffsetThisGame()) ;
	refPlayer["coin"] = (int32_t)pWritePlayer->getCoin() ;
	refPlayer["state"] = pWritePlayer->getState();
	m_arrPlayers[pWritePlayer->getIdx()] = refPlayer ;
	CLogMgr::SharedLogMgr()->PrintLog("write player uid = %d result log to json",pWritePlayer->getUserUID());
}

uint8_t CTaxasRoom::GetFirstInvalidIdxWithState( uint8_t nIdxFromInclude , eRoomPeerState estate )
{
	auto seatCnt = getSeatCount() ;
	for ( uint8_t nIdx = nIdxFromInclude ; nIdx < seatCnt * 2 ; ++nIdx )
	{
		uint8_t nRealIdx = nIdx % seatCnt ;
		if ( getPlayerByIdx(nRealIdx) == nullptr )
		{
			continue;
		}

		if ( getPlayerByIdx(nRealIdx)->isHaveState(estate) )
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
		CLogMgr::SharedLogMgr()->ErrorLog("why this pool coin is 0 ? players = %d room id = %d ",pPool.vInPoolPlayerIdx.size(),getRoomID()) ;
		return ;
	}

	if ( pPool.vInPoolPlayerIdx.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why pool coin = %I64d , peers is 0 room id = %d  ",pPool.nCoin,getRoomID() ) ;
	}

	// find winner ;
	if ( pPool.vInPoolPlayerIdx.size() == 1 )
	{
		uint8_t nPeerIdx = pPool.vInPoolPlayerIdx[0] ;
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nPeerIdx);
		if ( pPlayer == nullptr )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this winner idx is invalid = %d, system got coin = %I64d",nPeerIdx,pPool.nCoin ) ;
			return ;
		}

		if ( pPlayer->isHaveState(eRoomPeer_WaitCaculate) == false )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this winner idx state is invalid = %d, system got coin = %I64d",nPeerIdx,pPool.nCoin ) ;
			return ;
		}
		pPool.vWinnerIdxs.push_back( nPeerIdx ) ;
		pPlayer->addWinCoinThisGame(pPool.nCoin);
		return ;
	}

	// pk card
	if ( IsPublicDistributeFinish() == false || m_vSortByPeerCardsAsc.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("public is not finish how to pk card ? error room id = %d",getRoomID());
		return ;
	}

	CTaxasPlayer* pWiner = nullptr ;
	for ( int8_t nIdx = m_vSortByPeerCardsAsc.size() - 1 ; nIdx >= 0; --nIdx )
	{
		CTaxasPlayer* pData = (CTaxasPlayer*)m_vSortByPeerCardsAsc[nIdx];
		if ( pData == nullptr || pData->isHaveState( eRoomPeer_WaitCaculate ) == false )
		{
			continue; ;
		}

		if ( pPool.isPlayerInThisPool(pData->getIdx()) == false )
		{
			continue;
		}

		if ( pPool.vWinnerIdxs.empty() )
		{
			pPool.vWinnerIdxs.push_back(pData->getIdx()) ;
			pWiner = pData ;
			continue;
		}

		if ( pWiner->getPeerCard()->PK(pData->getPeerCard()) != 0 ) // not the same , means small 
		{
			break ;
		}

		pPool.vWinnerIdxs.push_back(pData->getIdx()) ;
	}

	// give coin 
	if ( pPool.vWinnerIdxs.empty() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why room id = %d pool idx = %d winner is empty , system got coin = %I64d ",getRoomID(),pPool.nIdx,pPool.nCoin ) ;
		return ;
	}

	uint8_t nElasCoin = uint8_t(pPool.nCoin % pPool.vWinnerIdxs.size()) ;
	pPool.nCoin -= nElasCoin ;
	if ( nElasCoin > 0 )
	{
		setProfit(nElasCoin + getProfit());
		CLogMgr::SharedLogMgr()->SystemLog("system got the elaps coin = %d, room id = %d , pool idx = %d ",nElasCoin,getRoomID(),pPool.nIdx ) ;
	}
	uint64_t nCoinPerWinner = pPool.nCoin / pPool.vWinnerIdxs.size() ;
	for ( uint8_t nIdx = 0 ; nIdx < pPool.vWinnerIdxs.size(); ++nIdx )
	{
		uint8_t nPeerIdx = pPool.vWinnerIdxs[nIdx];
		auto pPlayer = (CTaxasPlayer*)getPlayerByIdx(nPeerIdx);
		pPlayer->addWinCoinThisGame(nCoinPerWinner);
		CLogMgr::SharedLogMgr()->SystemLog("player use uid = %d win coin = %I64d , from pool idx = %d, room id = %d",pPlayer->getUserUID(),nCoinPerWinner,pPool.nIdx,getRoomID()) ;
	}
}

void CTaxasRoom::sendRoomInfoToPlayer(uint32_t nSessionID )
{
	// send base info 
	stMsgTaxasRoomInfoBase msgBaseInfo ;
	msgBaseInfo.eCurRoomState = getCurRoomState()->getStateID() ;
	msgBaseInfo.nOwnerUID = getOwnerUID() ;
	msgBaseInfo.nBankerIdx = m_nBankerIdx ;
	memcpy(msgBaseInfo.vRoomName,getRoomName(),strlen(getRoomName()));
	msgBaseInfo.nBigBlindIdx = m_nBigBlindIdx ;
	msgBaseInfo.nCurMainBetPool = m_nCurMainBetPool;
	msgBaseInfo.nCurWaitPlayerActionIdx = m_nCurWaitPlayerActionIdx ;
	msgBaseInfo.nLittleBlind = m_nLittleBlind ;
	msgBaseInfo.nLittleBlindIdx = m_nLittleBlindIdx ;
	msgBaseInfo.nMaxSeat = getSeatCount();
	msgBaseInfo.nMostBetCoinThisRound = m_nMostBetCoinThisRound ;
	msgBaseInfo.nRoomID = getRoomID() ;
	msgBaseInfo.nMiniTakeIn = m_nMinTakeIn ;
	msgBaseInfo.nMaxTakeIn = m_nMaxTakeIn ;
	msgBaseInfo.nDeskFee = getDeskFee();
	msgBaseInfo.nChatRoomID = getChatRoomID();
	memset(msgBaseInfo.vPublicCardNums,0,sizeof(msgBaseInfo.vPublicCardNums));
	if ( m_nPublicCardRound == 1 )
	{
		for ( uint8_t nidx = 0 ; nidx < 3 ; ++nidx )
		{
			msgBaseInfo.vPublicCardNums[nidx] = m_vPublicCardNums[nidx];
		}
	}
	else if ( 2 == m_nPublicCardRound )
	{
		for ( uint8_t nidx = 0 ; nidx < 4 ; ++nidx )
		{
			msgBaseInfo.vPublicCardNums[nidx] = m_vPublicCardNums[nidx];
		}
	}
	else if ( 3 == m_nPublicCardRound )
	{
		for ( uint8_t nidx = 0 ; nidx < 5 ; ++nidx )
		{
			msgBaseInfo.vPublicCardNums[nidx] = m_vPublicCardNums[nidx];
		}
	}

	sendMsgToPlayer(&msgBaseInfo,sizeof(msgBaseInfo),nSessionID) ;

	// send vice pool 
	if ( GetFirstCanUseVicePool().nIdx )
	{
		stMsgTaxasRoomInfoVicePool msgVicePool ;
		memset(msgVicePool.vVicePool,0,sizeof(msgVicePool.vVicePool)) ;
		for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
		{
			msgVicePool.vVicePool[nIdx] = m_vAllVicePools[nIdx].nCoin;
		}
		sendMsgToPlayer(&msgVicePool,sizeof(msgVicePool),nSessionID);
	}

	// send Player data 
	uint8_t nCnt = getPlayerCntWithState(eRoomPeer_SitDown) ;
	stMsgTaxasRoomInfoPlayerData msgPlayerData ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount() ; ++nIdx )
	{
		CTaxasPlayer* pPlayer = (CTaxasPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr )
		{
			continue;
		}
		msgPlayerData.bIsLast = (--nCnt <= 0 );
		msgPlayerData.eCurAct = pPlayer->getCurActType() ;
		msgPlayerData.nBetCoinThisRound = pPlayer->getBetCoinThisRound() ;
		msgPlayerData.nSeatIdx = pPlayer->getIdx() ;
		msgPlayerData.nStateFlag = pPlayer->getState() ;
		msgPlayerData.nTakeInMoney = pPlayer->getCoin() ;
		msgPlayerData.nUserUID = pPlayer->getUserUID() ;
		msgPlayerData.vHoldCard[0] = pPlayer->getPeerCardByIdx(0) ;
		msgPlayerData.vHoldCard[1] = pPlayer->getPeerCardByIdx(1) ;
		sendMsgToPlayer(&msgPlayerData,sizeof(msgPlayerData),nSessionID) ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("send room data to player ");
}

//void CTaxasRoom::syncPlayerDataToDataSvr( stTaxasPeerData& pPlayerData )
//{
//	// if player requesting coin , do not sync data ;
//	stMsgCrossServerRequest msgReq ;
//	msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
//	msgReq.nReqOrigID = GetRoomID();
//	msgReq.nTargetID = pPlayerData.nUserUID ;
//	msgReq.nRequestType = eCrossSvrReq_AddMoney ;
//	msgReq.nRequestSubType = eCrossSvrReqSub_TaxasStandUp ;
//	msgReq.vArg[0] = true ;
//	msgReq.vArg[1] = pPlayerData.nTakeInMoney;
//	CTaxasServerApp::SharedGameServerApp()->sendMsg(pPlayerData.nSessionID,(char*)&msgReq,sizeof(msgReq)) ;
//	/// and just after game result ;
//	if ( pPlayerData.nPlayTimes > 0 )
//	{
//		stMsgSyncTaxasPlayerData msg ;
//		msg.nUserUID = pPlayerData.nUserUID ;
//		msg.nPlayTimes = pPlayerData.nPlayTimes ;
//		msg.nWinTimes = pPlayerData.nWinTimes ;
//		msg.nSingleWinMost = pPlayerData.nSingleWinMost ;
//		memset(msg.vBestCard,0,sizeof(msg.vBestCard));
//		if ( pPlayerData.vBestCards[0] ) // vBestCard not empty
//		{
//			memcpy(msg.vBestCard,pPlayerData.vBestCards,sizeof(msg.vBestCard));
//		}
//		CTaxasServerApp::SharedGameServerApp()->sendMsg(GetRoomID(),(char*)&msg,sizeof(msg)) ;
//	}
//}