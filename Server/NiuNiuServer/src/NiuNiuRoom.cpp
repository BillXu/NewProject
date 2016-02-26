#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoomDistribute4CardState.h"
#include "NiuNiuRoomTryBankerState.h"
#include "NiuNiuRoomRandBankerState.h"
#include "NiuNiuRoomBetState.h"
#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoomGameResult.h"
#include "NiuNiuMessageDefine.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "AutoBuffer.h"
#include "CardPoker.h"
#include "NiuNiuRoomPlayerCaculateCardState.h"
#include "ServerStringTable.h"
#include <algorithm>
#include <json/json.h>
CNiuNiuRoom::CNiuNiuRoom()
{
	getPoker()->InitTaxasPoker() ;
}

bool CNiuNiuRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue )
{
	ISitableRoom::init(pConfig,nRoomID,vJsValue) ;
	m_nBaseBet = ((stNiuNiuRoomConfig*)pConfig)->nBaseBet;

	return true ;
}

void CNiuNiuRoom::prepareState()
{
	ISitableRoom::prepareState();
	// create room state ;
	IRoomState* vState[] = {
		new CNiuNiuRoomDistribute4CardState(),new CNiuNiuRoomTryBanker(),new CNiuNiuRoomRandBankerState(),
		new CNiuNiuRoomBetState(),new CNiuNiuRoomDistributeFinalCardState(),new CNiuNiuRoomStatePlayerCaculateCardState() ,new CNiuNiuRoomGameResultState()
	};
	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
}

void CNiuNiuRoom::serializationFromDB(uint32_t nRoomID , Json::Value& vJsValue )
{
	ISitableRoom::serializationFromDB(nRoomID,vJsValue);
	m_nBaseBet = vJsValue["baseBet"].asUInt();
}

void CNiuNiuRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	ISitableRoom::willSerializtionToDB(vOutJsValue);
	vOutJsValue["baseBet"] = m_nBaseBet ;
}

ISitableRoomPlayer* CNiuNiuRoom::doCreateSitableRoomPlayer()
{
	return new CNiuNiuRoomPlayer();
}

void CNiuNiuRoom::sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID )
{
	CNiuNiuServerApp::getInstance()->sendMsg(nSessionID,(char*)pmsg,nLen);
}

bool CNiuNiuRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( ISitableRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_REQUEST_ROOM_INFORM:
		{
			std::string strInform = getRewardDesc() ;
			stMsgRequestNiuNiuRoomInformRet msg ;

			msg.nLen = strlen(strInform.c_str() );
			if ( msg.nLen == 0 )
			{
				sendMsgToPlayer(&msg,sizeof(msg),nPlayerSessionID) ;
				return true ;
			}

			uint16_t nLen = sizeof(msg) + msg.nLen ;
			char* pBuffer = new char[nLen];
			memcpy(pBuffer,&msg,sizeof(msg));
			memcpy(pBuffer + sizeof(msg),strInform.c_str(),msg.nLen);
			sendMsgToPlayer((stMsg*)pBuffer,nLen,nPlayerSessionID) ;
			delete[] pBuffer ;
		}
		break;
	case MSG_NN_MODIFY_ROOM_NAME:
		{
			stMsgModifyNiuNiuRoomNameRet msgBack ;
			if ( isRoomAlive() == false )
			{
				msgBack.nRet = 2 ;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				return true;
			}
			stMsgModifyNiuNiuRoomName* pRet = (stMsgModifyNiuNiuRoomName*)prealMsg ;
			msgBack.nRet = 0 ;
			pRet->vNewRoomName[MAX_LEN_ROOM_NAME-1] = 0 ;
			setRoomName(pRet->vNewRoomName);
			sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
			return true;
		}
		break;
	case MSG_NN_REQUEST_ROOM_INFO:
		{
			sendRoomInfoToPlayer(nPlayerSessionID);
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CNiuNiuRoom::onPlayerWillStandUp( ISitableRoomPlayer* pPlayer )
{
	if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
	{
		pPlayer->addState(eRoomPeer_StandUp) ;
	}
	else
	{
		playerDoStandUp(pPlayer);
	}
}

void CNiuNiuRoom::onPlayerWillLeaveRoom( stStandPlayer* pPlayer )
{
	ISitableRoomPlayer* pSitdownPlayer = getSitdownPlayerBySessionID(pPlayer->nUserSessionID) ;
	if ( pSitdownPlayer && pSitdownPlayer->isHaveState(eRoomPeer_CanAct) )
	{
		pSitdownPlayer->addState(eRoomPeer_WillLeave) ;
	}
	else
	{
		playerDoLeaveRoom(pPlayer->nUserUID);
	}
}

bool CNiuNiuRoom::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue )
{
	if ( ISitableRoom::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}
	return false ;
}

bool CNiuNiuRoom::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( ISitableRoom::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}
	return false ;
}

void CNiuNiuRoom::sendRoomInfoToPlayer(uint32_t nSessionID)
{
	stMsgNNRoomInfo msgInfo ;
	msgInfo.nBankerBetTimes = m_nBetBottomTimes ;
	msgInfo.nBankerIdx = m_nBankerIdx ;
	msgInfo.nDeskFee =  getDeskFee() ;
	msgInfo.nBlind = getBaseBet() ;
	msgInfo.nBottomBet = getBaseBet();
	msgInfo.nChatRoomID = getChatRoomID() ;
	msgInfo.nPlayerCnt = getSitDownPlayerCount();
	msgInfo.nRoomID = getRoomID() ;
	msgInfo.nRoomState = getCurRoomState()->getStateID();
	
	CAutoBuffer auBuffer(sizeof(msgInfo) + sizeof(stNNRoomInfoPayerItem) * msgInfo.nPlayerCnt);
	auBuffer.addContent(&msgInfo,sizeof(msgInfo));

	uint8_t nSeatCount = getSeatCount();
	stNNRoomInfoPayerItem item ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCount ; ++nIdx )
	{
		CNiuNiuRoomPlayer* psit = (CNiuNiuRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( psit )
		{
			item.nBetTimes = psit->getBetTimes() ;
			item.nCoin = psit->getCoin() ;
			item.nIdx = psit->getIdx() ;
			item.nStateFlag = psit->getState() ;
			item.nUserUID = psit->getUserUID() ;
			for ( uint8_t nCardIdx = 0 ; nCardIdx < NIUNIU_HOLD_CARD_COUNT ; ++nCardIdx )
			{
				item.vHoldChard[nCardIdx] = psit->getCardByIdx(nCardIdx) ;
			}
			auBuffer.addContent(&item,sizeof(item)) ;
		}
	}

	sendMsgToPlayer((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID) ;
	CLogMgr::SharedLogMgr()->PrintLog("send room info to session id = %d, player cnt = %d ", nSessionID,msgInfo.nPlayerCnt) ;
}

void CNiuNiuRoom::onTimeSave(bool bRightNow)
{
	ISitableRoom::onTimeSave();
}

uint8_t CNiuNiuRoom::getMaxRate()
{
	return getReateByNiNiuType(CNiuNiuPeerCard::NiuNiuType::Niu_FiveSmall,10);
}

uint32_t CNiuNiuRoom::getBaseBet()
{
	return m_nBaseBet ;
}

uint64_t& CNiuNiuRoom::getBankCoinLimitForBet()
{
	return m_nBankerCoinLimitForBet;
}

void CNiuNiuRoom::setBankCoinLimitForBet( uint64_t nCoin )
{
	m_nBankerCoinLimitForBet = nCoin ;
}

uint8_t CNiuNiuRoom::getReateByNiNiuType(uint8_t nType , uint8_t nPoint )
{
	return 1 ;
}

uint64_t CNiuNiuRoom::getLeastCoinNeedForBeBanker( uint8_t nBankerTimes )
{
	return getBaseBet() * nBankerTimes * getMaxRate() * ( getPlayerCntWithState(eRoomPeer_CanAct) - 1 );
}

void CNiuNiuRoom::onGameWillBegin()
{
	ISitableRoom::onGameWillBegin();
	m_nBankerIdx = 0 ;
	m_nBankerCoinLimitForBet = 0 ;
	m_nBetBottomTimes = 0 ;
	getPoker()->RestAllPoker();
	CLogMgr::SharedLogMgr()->PrintLog("room game begin");
}

void CNiuNiuRoom::onGameDidEnd()
{
	stMsgSaveLog msgSaveLog ;
	msgSaveLog.nLogType = eLog_NiuNiuGameResult ;
	msgSaveLog.nTargetID = getRoomID() ;
	msgSaveLog.nJsonExtnerLen = 0 ;
	memset(msgSaveLog.vArg,0,sizeof(msgSaveLog.vArg));
	msgSaveLog.vArg[0] = getPlayerByIdx(m_nBankerIdx)->getUserUID();
	msgSaveLog.vArg[1] = m_nBetBottomTimes ;
	msgSaveLog.vArg[2] = getBaseBet() * m_nBetBottomTimes ;
	m_arrPlayers.clear() ;

	m_nBankerIdx = 0 ;
	m_nBankerCoinLimitForBet = 0 ;
	m_nBetBottomTimes = 0 ;

	uint8_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx )
	{
		ISitableRoomPlayer* pSitDown = getPlayerByIdx(nIdx) ;
		if ( pSitDown == nullptr )
		{
			continue;
		}

		// write to log 
		if ( pSitDown->isHaveState(eRoomPeer_CanAct) )
		{
			CNiuNiuRoomPlayer* pNiuPlayer = (CNiuNiuRoomPlayer*)pSitDown ;
			Json::Value refPlayer ;
			refPlayer["uid"] = pNiuPlayer->getUserUID() ;
			refPlayer["idx"] = pNiuPlayer->getIdx();
			refPlayer["betTimes"] = pNiuPlayer->getBetTimes() <= 0 ? 5 : pNiuPlayer->getBetTimes();
			refPlayer["card0"] = pNiuPlayer->getCardByIdx(0);
			refPlayer["card1"] = pNiuPlayer->getCardByIdx(1);
			refPlayer["card2"] = pNiuPlayer->getCardByIdx(2);
			refPlayer["card3"] = pNiuPlayer->getCardByIdx(3);
			refPlayer["card4"] = pNiuPlayer->getCardByIdx(4);
			refPlayer["offset"] = 0 ;//pNiuPlayer->getCoinOffsetThisGame() ;
			refPlayer["coin"] = (int32_t)pNiuPlayer->getCoin() ;
			m_arrPlayers[pNiuPlayer->getIdx()] = refPlayer ;
		}

		pSitDown->removeState(eRoomPeer_CanAct);

		if ( pSitDown->isHaveState(eRoomPeer_WillLeave) )
		{
			CLogMgr::SharedLogMgr()->PrintLog("game end ,player uid = %d should leave ",pSitDown->getUserUID()) ;
			playerDoLeaveRoom(pSitDown->getUserUID());
		}
		else if ( pSitDown->isHaveState(eRoomPeer_StandUp) )
		{
			CLogMgr::SharedLogMgr()->PrintLog(" game end player uid = %d should stand up  ",pSitDown->getUserUID()) ;
			playerDoStandUp(pSitDown);
		}
		else
		{
			pSitDown->onGameEnd();
		}
	}

	Json::StyledWriter write ;
	std::string str = write.write(m_arrPlayers);
	CAutoBuffer auBuffer (sizeof(msgSaveLog) + str.size());
	msgSaveLog.nJsonExtnerLen = str.size() ;
	auBuffer.addContent((char*)&msgSaveLog,sizeof(msgSaveLog)) ;
	auBuffer.addContent(str.c_str(),str.size());
	sendMsgToPlayer((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),getRoomID()) ;

	ISitableRoom::onGameDidEnd();
	CLogMgr::SharedLogMgr()->PrintLog("room game End");
}

bool sortPlayerByCard(ISitableRoomPlayer* pLeft , ISitableRoomPlayer* pRight )
{
	CNiuNiuRoomPlayer* pNLeft = (CNiuNiuRoomPlayer*)pLeft ;
	CNiuNiuRoomPlayer* pNRight = (CNiuNiuRoomPlayer*)pRight ;
	if ( pNLeft->getPeerCard()->pk(pNRight->getPeerCard()) == IPeerCard::PK_RESULT_FAILED )
	{
		return true ;
	}
	return false ;
}

void CNiuNiuRoom::prepareCards()
{
	// parepare cards for all player ;
	uint8_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0 ; nIdx < nSeatCnt ; ++nIdx )
	{
		CNiuNiuRoomPlayer* pRoomPlayer = (CNiuNiuRoomPlayer*)getPlayerByIdx(nIdx) ;
		if ( pRoomPlayer && pRoomPlayer->isHaveState(eRoomPeer_CanAct))
		{
			uint8_t nCardCount = NIUNIU_HOLD_CARD_COUNT ;
			uint8_t nCardIdx = 0 ;
			while ( nCardIdx < nCardCount )
			{
				pRoomPlayer->onGetCard(nCardIdx,getPoker()->GetCardWithCompositeNum()) ;
				++nCardIdx ;
			}
			m_vSortByPeerCardsAsc.push_back(pRoomPlayer) ;
		}
	}

	std::sort(m_vSortByPeerCardsAsc.begin(),m_vSortByPeerCardsAsc.end(),sortPlayerByCard);

	doProcessNewPlayerHalo();
}

uint32_t CNiuNiuRoom::coinNeededToSitDown()
{
	return getBaseBet()* 4 * getMaxRate() * 25 * 2 + getDeskFee() ;
}

void CNiuNiuRoom::caculateGameResult()
{
	// caculate result ;
	CNiuNiuRoomPlayer* pBanker = (CNiuNiuRoomPlayer*)getPlayerByIdx(getBankerIdx()) ;
	assert(pBanker && "why banker is null ?");
	CLogMgr::SharedLogMgr()->PrintLog("banker coin = %I64d",pBanker->getCoin()) ;

	// send result msg ;
	stMsgNNGameResult msgResult ;
	msgResult.nPlayerCnt = m_vSortByPeerCardsAsc.size() ;

	CAutoBuffer auBuffer(sizeof(msgResult) + msgResult.nPlayerCnt * sizeof(stNNGameResultItem));
	auBuffer.addContent(&msgResult,sizeof(msgResult)) ;

	int64_t nBankerOffset = 0 ;
	// caclulate banker win ;
	for ( ISitableRoomPlayer* pPlayer : m_vSortByPeerCardsAsc )
	{
		CNiuNiuRoomPlayer* pNNP = (CNiuNiuRoomPlayer*)pPlayer ;
		if ( pNNP == pBanker )
		{
			break;
		}

		int64_t nLoseCoin = pNNP->getBetTimes() * getBaseBet() * m_nBetBottomTimes ;
		if ( nLoseCoin > pNNP->getCoin() )
		{
			nLoseCoin = pNNP->getCoin() ;
			CLogMgr::SharedLogMgr()->ErrorLog("you do not have coin why you bet so many coin , uid = %d",pNNP->getUserUID());
		}

		nBankerOffset += nLoseCoin ;
		pNNP->setCoin(pNNP->getCoin() - nLoseCoin ) ;
		pBanker->setCoin(pBanker->getCoin() + nLoseCoin) ;

		stNNGameResultItem item ;
		item.nFinalCoin = pNNP->getCoin() ;
		item.nOffsetCoin = -1* nLoseCoin ;
		item.nPlayerIdx = pNNP->getIdx() ;
		auBuffer.addContent(&item,sizeof(item)) ;
		updatePlayerOffset(pNNP->getUserUID(),item.nOffsetCoin) ;
	}

	// caculate banker lose 
	for ( uint8_t nIdx = m_vSortByPeerCardsAsc.size() -1 ; nIdx >= 0 ; --nIdx )
	{
		CNiuNiuRoomPlayer* pNNP = (CNiuNiuRoomPlayer*)m_vSortByPeerCardsAsc[nIdx] ;
		if ( pNNP == pBanker )
		{
			break;
		}

		int64_t nBankerLoseCoin = pNNP->getBetTimes() * getBaseBet() * m_nBetBottomTimes ;
		if ( nBankerLoseCoin > pBanker->getCoin() )
		{
			nBankerLoseCoin = pBanker->getCoin() ;
		}

		nBankerOffset -= nBankerLoseCoin ;
		pBanker->setCoin(pBanker->getCoin() - nBankerLoseCoin ) ;
		pNNP->setCoin(pNNP->getCoin() + nBankerLoseCoin ) ;

		stNNGameResultItem item ;
		item.nFinalCoin = pNNP->getCoin() ;
		item.nOffsetCoin = nBankerLoseCoin ;
		item.nPlayerIdx = pNNP->getIdx() ;
		auBuffer.addContent(&item,sizeof(item)) ;
		updatePlayerOffset(pNNP->getUserUID(),item.nOffsetCoin) ;
	}

	stNNGameResultItem item ;
	item.nFinalCoin = pBanker->getCoin() ;
	item.nOffsetCoin = nBankerOffset ;
	item.nPlayerIdx = pBanker->getIdx() ;
	auBuffer.addContent(&item,sizeof(item)) ;
	updatePlayerOffset(pBanker->getUserUID(),item.nOffsetCoin) ;
	CLogMgr::SharedLogMgr()->PrintLog("result player idx = %d , finalCoin = %llu, offset coin = %I64d",item.nPlayerIdx,item.nFinalCoin,item.nOffsetCoin) ;

	sendRoomMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize()) ;
}

bool CNiuNiuRoom::canStartGame()
{
	return getPlayerCntWithState(eRoomPeer_CanAct) >= 2 ;
}