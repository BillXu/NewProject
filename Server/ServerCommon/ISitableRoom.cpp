#include "ISitableRoom.h"
#include "RoomConfig.h"
#include "ISitableRoomPlayer.h"
#include <cassert>
#include "MessageDefine.h"
#include "AutoBuffer.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include <json/json.h>
#include "ServerStringTable.h"
#include "SeverUtility.h"
#include <time.h>
#include "IRoomDelegate.h"
#include <algorithm>
#include "IPeerCard.h"
ISitableRoom::~ISitableRoom()
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx])
		{
			delete m_vSitdownPlayers[nIdx];
			m_vSitdownPlayers[nIdx] = nullptr ;
		}
	}
	delete [] m_vSitdownPlayers;

	for ( ISitableRoomPlayer* pPlayer : m_vReserveSitDownObject )
	{
		delete pPlayer ;
		pPlayer = nullptr ;
	}
	m_vReserveSitDownObject.clear() ;
}

bool ISitableRoom::onFirstBeCreated(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue ) 
{
	IRoom::onFirstBeCreated(pRoomMgr,pConfig,nRoomID,vJsValue) ;
	stSitableRoomConfig* pC = (stSitableRoomConfig*)pConfig;
	m_nSeatCnt = pC->nMaxSeat ;
	m_vSitdownPlayers = new ISitableRoomPlayer*[m_nSeatCnt] ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		m_vSitdownPlayers[nIdx] = nullptr ;
	}
	return true ;
}

void ISitableRoom::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	IRoom::serializationFromDB(pRoomMgr,pConfig,nRoomID,vJsValue);
	stSitableRoomConfig* pC = (stSitableRoomConfig*)pConfig;
	m_nSeatCnt = pC->nMaxSeat ;
	m_vSitdownPlayers = new ISitableRoomPlayer*[m_nSeatCnt] ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		m_vSitdownPlayers[nIdx] = nullptr ;
	}
}

void ISitableRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	IRoom::willSerializtionToDB(vOutJsValue);
}

void ISitableRoom::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	IRoom::roomItemDetailVisitor(vOutJsValue);
	vOutJsValue["playerCnt"] = getSitDownPlayerCount();
}

bool ISitableRoom::canStartGame()
{
	if ( IRoom::canStartGame() == false )
	{
		return false ;
	}

	return getPlayerCntWithState(eRoomPeer_WaitNextGame) >= 2 ;
}

//bool ISitableRoom::onPlayerSitDown(ISitableRoomPlayer* pPlayer , uint8_t nIdx )
//{
//	if ( isSeatIdxEmpty(nIdx) )
//	{
//		m_vSitdownPlayers[nIdx] = pPlayer ;
//		pPlayer->doSitdown(nIdx) ;
//		pPlayer->setIdx(nIdx);
//
//		// save standup log ;
//		stMsgSaveLog msgLog ;
//		msgLog.nJsonExtnerLen = 0 ;
//		msgLog.nLogType = eLog_PlayerSitDown ;
//		msgLog.nTargetID = pPlayer->getUserUID() ;
//		memset(msgLog.vArg,0,sizeof(msgLog.vArg)) ;
//		msgLog.vArg[0] = getRoomType() ;
//		msgLog.vArg[1] = getRoomID() ;
//		msgLog.vArg[2] = pPlayer->getCoin() ;
//		sendMsgToPlayer(&msgLog,sizeof(msgLog),getRoomID()) ;
//		return true ;
//	}
//	return false ;
//}

void ISitableRoom::playerDoStandUp( ISitableRoomPlayer* pPlayer )
{
	// remove from m_vSortByPeerCardsAsc ;
	auto iterSort = m_vSortByPeerCardsAsc.begin() ;
	for ( ; iterSort != m_vSortByPeerCardsAsc.end(); ++iterSort )
	{
		if ( *iterSort == pPlayer )
		{
			m_vSortByPeerCardsAsc.erase(iterSort) ;
			break;
		}
	}

	// remove other player data ;
	assert(isSeatIdxEmpty(pPlayer->getIdx()) == false && "player not sit down" );
	pPlayer->willStandUp();
	m_vSitdownPlayers[pPlayer->getIdx()] = nullptr ;
	auto standPlayer = getPlayerByUserUID(pPlayer->getUserUID()) ;
	
	if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) )
	{
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d invoke game end , before stand up",pPlayer->getUserUID()) ;
		pPlayer->onGameEnd();
	}

	if ( standPlayer == nullptr )
	{
		if ( pPlayer->getCoin() > 0 )
		{
			stMsgSvrDelayedLeaveRoom msgdoLeave ;
			msgdoLeave.nCoin = pPlayer->getCoin() ;
			msgdoLeave.nGameType = getRoomType() ;
			msgdoLeave.nRoomID = getRoomID() ;
			msgdoLeave.nUserUID = pPlayer->getUserUID() ;
			msgdoLeave.nWinTimes = pPlayer->getWinTimes()  ;
			msgdoLeave.nPlayerTimes = pPlayer->getPlayTimes() ;
			msgdoLeave.nSingleWinMost = pPlayer->getSingleWinMost() ;
			msgdoLeave.nUserUID = pPlayer->getUserUID() ;
			msgdoLeave.nGameOffset = pPlayer->getTotalGameOffset() ;
			sendMsgToPlayer(&msgdoLeave,sizeof(msgdoLeave),pPlayer->getSessionID()) ;
			CLogMgr::SharedLogMgr()->PrintLog("player uid = %d game end stand up sys coin = %d to data svr ",pPlayer->getUserUID(),pPlayer->getCoin()) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->PrintLog("player uid = %d just stand up dely leave , but no coin",pPlayer->getUserUID() ) ;
		}
	}
	else
	{
		standPlayer->nCoin += pPlayer->getCoin() ;
		standPlayer->nNewPlayerHaloWeight = pPlayer->getHaloWeight() ;
		standPlayer->nPlayerTimes += pPlayer->getPlayTimes();
		standPlayer->nWinTimes += pPlayer->getWinTimes();
		standPlayer->nGameOffset += pPlayer->getTotalGameOffset() ;
		if ( pPlayer->getSingleWinMost() > standPlayer->nSingleWinMost )
		{
			standPlayer->nSingleWinMost = pPlayer->getSingleWinMost() ;
		}
		CLogMgr::SharedLogMgr()->PrintLog("player uid = %d just normal stand up ",pPlayer->getUserUID() ) ;
	}

	stMsgRoomStandUp msgStandUp ;
	msgStandUp.nIdx = pPlayer->getIdx() ;
	sendRoomMsg(&msgStandUp,sizeof(msgStandUp));

	m_vReserveSitDownObject.push_back(pPlayer) ;
}

void ISitableRoom::onPlayerWillLeaveRoom(stStandPlayer* pPlayer )
{
	ISitableRoomPlayer* pSitPlayer = getSitdownPlayerByUID(pPlayer->nUserUID) ;
	if ( pSitPlayer == nullptr )
	{
		return ;
	}

	onPlayerWillStandUp(pSitPlayer);
	pSitPlayer = getSitdownPlayerByUID(pPlayer->nUserUID) ;
	if ( pSitPlayer == nullptr )
	{
		CLogMgr::SharedLogMgr()->PrintLog("player direct standup and can leave uid = %u",pPlayer->nUserUID) ;
		return ;
	}

	uint32_t nLeastLeftCoin = getLeastCoinNeedForCurrentGameRound(pSitPlayer) ;
	uint32_t nCoin = pSitPlayer->getCoin() ;
	if ( nCoin > nLeastLeftCoin )
	{
		pSitPlayer->setCoin(nLeastLeftCoin) ;
		pPlayer->nCoin += ( nCoin - nLeastLeftCoin );
		CLogMgr::SharedLogMgr()->PrintLog("uid = %u will leave take away coin = %u, left coin = %u",pPlayer->nUserUID,pPlayer->nCoin,pSitPlayer->getCoin() ) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->PrintLog("need coin too many ,uid = %u will leave take away coin = %u, left coin = %u",pPlayer->nUserUID,pPlayer->nCoin,pSitPlayer->getCoin() ) ;
	}
}

void ISitableRoom::onPlayerWillStandUp(ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer )
	{
		pPlayer->delayStandUp();
	}
}

uint16_t ISitableRoom::getEmptySeatCount()
{
	uint16_t nCount = 0 ;
	for ( uint16_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] == nullptr )
		{
			++nCount ;
		}
	}
	return nCount ;
}

ISitableRoomPlayer* ISitableRoom::getPlayerByIdx(uint16_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return nullptr ;
	}
	return m_vSitdownPlayers[nIdx] ;
}

bool ISitableRoom::isSeatIdxEmpty( uint8_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return false ;
	}
	return m_vSitdownPlayers[nIdx] == nullptr ;
}

uint16_t ISitableRoom::getSitDownPlayerCount()
{
	return getSeatCount() - getEmptySeatCount() ;
}

uint16_t ISitableRoom::getSeatCount()
{
	return m_nSeatCnt ;
}

ISitableRoomPlayer* ISitableRoom::getReuseSitableRoomPlayerObject()
{
	LIST_SITDOWN_PLAYERS::iterator iter = m_vReserveSitDownObject.begin() ;
	if ( iter != m_vReserveSitDownObject.end() )
	{
		ISitableRoomPlayer* p = *iter ;
		m_vReserveSitDownObject.erase(iter) ;
		return p ;
	}
	return doCreateSitableRoomPlayer();
}

uint16_t ISitableRoom::getPlayerCntWithState( uint32_t nState )
{
	uint8_t nCount = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->isHaveState(nState) )
		{
			++nCount ;
		}
	}
	return nCount ;
}

ISitableRoomPlayer* ISitableRoom::getSitdownPlayerBySessionID(uint32_t nSessionID)
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->getSessionID() == nSessionID )
		{
			return m_vSitdownPlayers[nIdx] ;
		}
	}
	return nullptr ;
}

ISitableRoomPlayer* ISitableRoom::getSitdownPlayerByUID(uint32_t nUserUID )
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->getUserUID() == nUserUID )
		{
			return m_vSitdownPlayers[nIdx] ;
		}
	}
	return nullptr ;
}

bool ISitableRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_ADD_TEMP_HALO:
		{
			auto pPlayer = getPlayerBySessionID(nPlayerSessionID) ;
			if ( pPlayer == nullptr )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("not in room player add temp halo session id = %u",nPlayerSessionID);
				break;
			}

			//if ( pPlayer->nPlayerType == ePlayer_Normal )
			//{
			//	CLogMgr::SharedLogMgr()->ErrorLog("normal can not add temp halo");
			//	break;
			//}

			stMsgAddTempHalo* pRet = (stMsgAddTempHalo*)prealMsg ;
			if ( 0 == pRet->nTargetUID )
			{
				pRet->nTargetUID = pPlayer->nUserUID ;
			}

			auto psitpp = getSitdownPlayerByUID(pRet->nTargetUID) ;
			if ( psitpp )
			{
				psitpp->setTempHaloWeight(pRet->nTempHalo);
				CLogMgr::SharedLogMgr()->PrintLog("uid = %u add temp halo = %u",pRet->nTargetUID,pRet->nTempHalo) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %u not sit down why add temp halo",pPlayer->nUserUID);
			}
		}
		break;
	case MSG_PLAYER_SITDOWN:
		{
			stMsgPlayerSitDownRet msgBack ;
			msgBack.nRet = 0 ;

			stStandPlayer* pPlayer = getPlayerBySessionID(nPlayerSessionID) ;
			if ( !pPlayer )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("palyer session id = %d ,not in this room so , can not sit down",nPlayerSessionID) ;
				msgBack.nRet = 3 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}

			auto pp = getSitdownPlayerByUID(pPlayer->nUserUID);
			if ( pp )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("session id = %d , already sit down , don't sit down again",nPlayerSessionID ) ;
				msgBack.nRet = 4 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break;
			}

			stMsgPlayerSitDown* pRet = (stMsgPlayerSitDown*)prealMsg ;
			if ( pRet->nTakeInCoin == 0 || pRet->nTakeInCoin > pPlayer->nCoin)
			{
				pRet->nTakeInCoin = pPlayer->nCoin ;
			}

			if ( pRet->nTakeInCoin < coinNeededToSitDown() )
			{
				msgBack.nRet = 1 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}

			if ( isSeatIdxEmpty(pRet->nIdx) == false )
			{
				msgBack.nRet = 2 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}

			auto sitDownPlayer = getReuseSitableRoomPlayerObject() ;
			sitDownPlayer->reset(pPlayer) ;
			pPlayer->nCoin -= pRet->nTakeInCoin ;
			sitDownPlayer->setCoin(pRet->nTakeInCoin) ;
			sitDownPlayer->doSitdown(pRet->nIdx) ;
			sitDownPlayer->setIdx(pRet->nIdx);
			sitDownPlayer->setState(eRoomPeer_WaitNextGame );
			m_vSitdownPlayers[pRet->nIdx] = sitDownPlayer ;

			// tell others ;
			stMsgRoomSitDown msgSitDown ;
			msgSitDown.nIdx = sitDownPlayer->getIdx() ;
			msgSitDown.nSitDownPlayerUserUID = sitDownPlayer->getUserUID() ;
			msgSitDown.nTakeInCoin = sitDownPlayer->getCoin() ;
			sendRoomMsg(&msgSitDown,sizeof(msgSitDown));

			onPlayerSitDown(sitDownPlayer) ;
		}
		break;
	case MSG_PLAYER_STANDUP:
		{
			stMsgPlayerStandUpRet msgBack ;
			msgBack.nRet = 0 ;
			auto player = getSitdownPlayerBySessionID(nPlayerSessionID) ;
			if ( player == nullptr )
			{
				msgBack.nRet = 1 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				break; 
			}
			onPlayerWillStandUp(player);
		}
		break;
	default:
		return false;
	}
	return true ;
}

void ISitableRoom::onGameDidEnd()
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		auto pPlayer = m_vSitdownPlayers[nIdx] ;
		if ( pPlayer == nullptr )
		{
			continue;
		}

		if ( pPlayer->isHaveState(eRoomPeer_StayThisRound) && getDelegate() )
		{
			getDelegate()->onUpdatePlayerGameResult(this,pPlayer->getUserUID(),pPlayer->getGameOffset()) ;
			CLogMgr::SharedLogMgr()->PrintLog("update room peer offset uid = %u, offset = %d",pPlayer->getUserUID(),pPlayer->getGameOffset());
		}

		if ( (pPlayer->isDelayStandUp() || (getDelegate() && getDelegate()->isPlayerLoseReachMax(this,pPlayer->getUserUID())) ) )
		{
			playerDoStandUp(pPlayer);	
			pPlayer = nullptr ;
			m_vSitdownPlayers[nIdx] = nullptr ;
		}

		if ( pPlayer->getCoin() < coinNeededToSitDown() )
		{
			playerDoStandUp(pPlayer);	
			pPlayer = nullptr ;
			m_vSitdownPlayers[nIdx] = nullptr ;
		}

		if ( pPlayer )
		{
			pPlayer->onGameEnd() ;
		}
	}
	m_vSortByPeerCardsAsc.clear();
	IRoom::onGameDidEnd() ;
}

void ISitableRoom::onGameWillBegin()
{
	IRoom::onGameWillBegin() ;
	uint16_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx )
	{
		ISitableRoomPlayer* pp = getPlayerByIdx(nIdx) ;
		if ( pp )
		{
			pp->setCoin(pp->getCoin() - getDeskFee() ) ;
			addTotoalProfit(getDeskFee());
			pp->onGameBegin();
		}
	}
}

bool sortPlayerByCard(ISitableRoomPlayer* pLeft , ISitableRoomPlayer* pRight )
{
	if ( pLeft->getPeerCard()->pk(pRight->getPeerCard()) == IPeerCard::PK_RESULT_FAILED )
	{
		return true ;
	}
	return false ;
}

void ISitableRoom::doProcessNewPlayerHalo()
{
	// add peer 
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		auto pPlayer = m_vSitdownPlayers[nIdx] ;
		if ( pPlayer && (pPlayer->isHaveState(eRoomPeer_CanAct)) )
		{
			m_vSortByPeerCardsAsc.push_back(pPlayer) ;
		}
	}

	if ( m_vSortByPeerCardsAsc.size() < 2 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why can act player count not bigger than 2 room id = %u",getRoomID()) ;
		return ;
	}

	// sort by peer card 
	std::sort(m_vSortByPeerCardsAsc.begin(),m_vSortByPeerCardsAsc.end(),sortPlayerByCard);
	//if ( isOmitNewPlayerHalo() )
	//{
	//	return ;
	//}

	// process halo 
	uint8_t nHalfCnt = m_vSortByPeerCardsAsc.size() ;
	uint8_t nSwitchTargetIdx = m_vSortByPeerCardsAsc.size() - 1 ;
	for ( uint8_t nIdx = 0 ; nIdx < nHalfCnt; ++nIdx)
	{
		if ( m_vSortByPeerCardsAsc[nIdx]->isHaveHalo() == false )
		{
			continue;
		}

		for ( ; nSwitchTargetIdx > nIdx ; --nSwitchTargetIdx )
		{
			if ( m_vSortByPeerCardsAsc[nSwitchTargetIdx]->isHaveHalo() )
			{
				continue;
			}

			m_vSortByPeerCardsAsc[nIdx]->switchPeerCard(m_vSortByPeerCardsAsc[nSwitchTargetIdx]);
			auto player = m_vSortByPeerCardsAsc[nIdx] ;
			m_vSortByPeerCardsAsc[nIdx] = m_vSortByPeerCardsAsc[nSwitchTargetIdx] ;
			m_vSortByPeerCardsAsc[nSwitchTargetIdx] = player ;

			if ( nSwitchTargetIdx == 0 )
			{
				return ;
			}
			--nSwitchTargetIdx;
			break;
		}
	}
#ifndef NDEBUG
	CLogMgr::SharedLogMgr()->PrintLog("room id = %u do halo result:",getRoomID());
	for ( uint8_t nIdx = 0 ; nIdx < m_vSortByPeerCardsAsc.size() ; ++nIdx )
	{
		CLogMgr::SharedLogMgr()->PrintLog("idx = %u uid = %u",nIdx,m_vSortByPeerCardsAsc[nIdx]->getUserUID());
	}
	CLogMgr::SharedLogMgr()->PrintLog("room id = %u halo end",getRoomID());
#endif // !NDEBUG

}

uint8_t ISitableRoom::GetFirstInvalidIdxWithState( uint8_t nIdxFromInclude , eRoomPeerState estate )
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
