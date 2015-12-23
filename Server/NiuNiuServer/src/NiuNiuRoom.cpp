#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoomWaitJoinState.h"
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
bool CNiuNiuRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID )
{
	ISitableRoom::init(pConfig,nRoomID) ;
	// create room state ;
	IRoomState* vState[] = {
							new CNiuNiuRoomWaitJoinState() , new CNiuNiuRoomDistribute4CardState(),new CNiuNiuRoomTryBanker(),new CNiuNiuRoomRandBankerState(),
							new CNiuNiuRoomBetState(),new CNiuNiuRoomDistributeFinalCardState(),new CNiuNiuRoomStatePlayerCaculateCardState() ,new CNiuNiuRoomGameResultState()
							};
	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
	getPoker()->InitTaxasPoker() ;
	// set default room state ;
	setInitState(vState[0]) ;
	return true ;
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
	case MSG_NN_REQUEST_ROOM_INFO:
		{
			sendRoomInfoToPlayer(nPlayerSessionID);
		}
		break;
	case MSG_NN_LEAVE_ROOM:
		{
			IRoomPlayer* pp = getPlayerBySessionID(nPlayerSessionID) ;
			if ( pp )
			{
				CLogMgr::SharedLogMgr()->PrintLog("MSG_NN_LEAVE_ROOM cur session id = %d  uid = %d , i let you leave",pp->getSessionID(),pp->getUserUID() );
				ISitableRoomPlayer* pPlayer = getSitdownPlayerBySessionID(pp->getSessionID()) ;
				if ( pPlayer )
				{
					pPlayer->addState(eRoomPeer_WillLeave) ;
				}

				if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
				{
					CLogMgr::SharedLogMgr()->PrintLog("player uid = %d apply leave ,direct do leave ",pp->getUserUID()) ;
					doPlayerLeave(pp->getSessionID()) ;
				}
				else
				{
					CLogMgr::SharedLogMgr()->PrintLog("player uid = %d apply leave ,but pls wait game end ",pp->getUserUID()) ;
				}
			}
			else
			{
				CLogMgr::SharedLogMgr()->PrintLog("you are not in room how do you send me leave msg") ;
			}
		}
		break;
	case MSG_NN_PLAYER_SITDOWN:
		{
			stMsgNNPlayerSitDown* msgPlayerSitdown = (stMsgNNPlayerSitDown*)prealMsg;
			stMsgNNPlayerSitDownRet msgBack ;
			if ( isSeatIdxEmpty(msgPlayerSitdown->nSeatIdx) == false )
			{
				msgBack.nRet = 1;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				return true ;
			}

			IRoomPlayer* pp = getPlayerBySessionID(nPlayerSessionID) ;
			if ( pp == nullptr )
			{
				msgBack.nRet = 3;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				return true ;
			}

			ISitableRoomPlayer* pSitDownTwice = getSitdownPlayerBySessionID(nPlayerSessionID) ;
			if ( pSitDownTwice )
			{
				msgBack.nRet = 4;
				sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;
				return true ;
			}

			CNiuNiuRoomPlayer* pNNPlayer = (CNiuNiuRoomPlayer*)getReuseSitableRoomPlayerObject();
			pNNPlayer->setUserUID(pp->getUserUID()) ;
			pNNPlayer->setSessionID(pp->getSessionID()) ;
			pNNPlayer->setCoin(pp->getCoin()) ;
			pNNPlayer->setInitCoin(pp->getCoin()) ;
			pNNPlayer->setState(eRoomPeer_WaitNextGame) ;
			playerSitDown(pNNPlayer,msgPlayerSitdown->nSeatIdx) ;

			msgBack.nRet = 0;
			sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID) ;

			// tell other player 
			stMsgNNSitDown msgSitdown ;
			memset(&msgSitdown.tSitDownPlayer,0,sizeof(msgSitdown.tSitDownPlayer)) ;
			msgSitdown.tSitDownPlayer.nCoin = pNNPlayer->getCoin();
			msgSitdown.tSitDownPlayer.nIdx = pNNPlayer->getIdx() ;
			msgSitdown.tSitDownPlayer.nUserUID = pNNPlayer->getUserUID() ;
			msgSitdown.tSitDownPlayer.nStateFlag = eRoomPeer_WaitNextGame ;
			sendRoomMsg(&msgSitdown,sizeof(msgSitdown)) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d sit down , idx = %d, coin = %llu",msgSitdown.tSitDownPlayer.nUserUID,msgSitdown.tSitDownPlayer.nIdx,pNNPlayer->getCoin());
		}
		break;
	case MSG_NN_PLAYER_STANDUP:
		{
			stMsgNNPlayerStandUp* pRet = (stMsgNNPlayerStandUp*)prealMsg ;
			CNiuNiuRoomPlayer* pSitDown = (CNiuNiuRoomPlayer*)getSitdownPlayerBySessionID(nPlayerSessionID) ;
			if ( pSitDown == nullptr )
			{
				return true ;
			}

			pSitDown->addState(eRoomPeer_StandUp) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d apply to stand up",pSitDown->getUserUID()) ;	
			if ( ! pSitDown->isHaveState(eRoomPeer_CanAct) )
			{
				playerStandUp(pSitDown);
			}
					
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CNiuNiuRoom::playerStandUp( ISitableRoomPlayer* pSitDown )
{
	if ( pSitDown->isHaveState(eRoomPeer_StandUp ) )
	{
		stMsgNNStandUp stUp ;
		stUp.nPlayerIdx = pSitDown->getIdx() ;
		sendRoomMsg(&stUp,sizeof(stUp));

		CLogMgr::SharedLogMgr()->PrintLog("payer uid = %d do stand up",pSitDown->getUserUID() );
		IRoomPlayer* pBaseRoomPeer = getPlayerBySessionID(pSitDown->getSessionID()) ;
		assert(pBaseRoomPeer && "object must not null");
		pBaseRoomPeer->setCoin(pSitDown->getCoin()) ;
		// sync coin to data svr ;
		stMsgCrossServerRequest msgSyncMoney ;
		msgSyncMoney.nJsonsLen = 0 ;
		msgSyncMoney.cSysIdentifer = ID_MSG_PORT_DATA ;
		msgSyncMoney.nReqOrigID = getRoomID() ;
		msgSyncMoney.nRequestSubType = eCrossSvrReqSub_Default;
		msgSyncMoney.nRequestType = eCrossSvrReq_SyncCoin ;
		msgSyncMoney.nTargetID = pBaseRoomPeer->getUserUID() ;
		msgSyncMoney.vArg[0] = pBaseRoomPeer->getCoin();
		msgSyncMoney.vArg[1] = eRoom_NiuNiu ;
		sendMsgToPlayer(&msgSyncMoney,sizeof(msgSyncMoney),getRoomID()) ;

		// so some rank in room recorder ;
		updatePlayerOffset(pSitDown->getUserUID(),pSitDown->getCoin() - pSitDown->getInitCoin() ) ;

		ISitableRoom::playerStandUp(pSitDown) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("do not have stanup flag ,so can not stand up");
		pSitDown->addState(eRoomPeer_StandUp);
	}
}

bool CNiuNiuRoom::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue )
{
	if ( ISitableRoom::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_ApplyLeaveRoom:
		{
			IRoomPlayer* pp = getPlayerByUserUID(pRequest->nReqOrigID) ;
			if ( pp )
			{
				CLogMgr::SharedLogMgr()->PrintLog("ApplyLeaveRoom cur session id = %d , old sessionid = %d , uid = %d , i let you leave",(uint32_t)pRequest->vArg[1],pp->getSessionID(),pRequest->nReqOrigID);
				ISitableRoomPlayer* pPlayer = getSitdownPlayerBySessionID(pp->getSessionID()) ;
				if ( pPlayer )
				{
					pPlayer->addState(eRoomPeer_WillLeave) ;
				}

				if ( pPlayer == nullptr || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
				{
					CLogMgr::SharedLogMgr()->PrintLog("player uid = %d apply leave ,direct do leave ",pRequest->nReqOrigID) ;
					doPlayerLeave(pp->getSessionID()) ;
				}
				else
				{
					CLogMgr::SharedLogMgr()->PrintLog("player uid = %d apply leave ,but pls wait game end ",pRequest->nReqOrigID) ;
				}
			}
			else
			{
				stMsgCrossServerRequest msgEnter ;
				msgEnter.cSysIdentifer = ID_MSG_PORT_DATA ;
				msgEnter.nJsonsLen = 0 ;
				msgEnter.nReqOrigID = getRoomID();
				msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
				msgEnter.nRequestType = eCrossSvrReq_LeaveRoomRet ;
				msgEnter.nTargetID = pRequest->nReqOrigID ;
				msgEnter.vArg[0] = eRoom_NiuNiu ;
				msgEnter.vArg[1] = getRoomID() ;
				sendMsgToPlayer(&msgEnter,sizeof(msgEnter),getRoomID()) ;
				CLogMgr::SharedLogMgr()->PrintLog("you are not in room but i let you go!") ;
			}
		}
		break;
	default:
		return false;
	}
	return true ;
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

IRoomPlayer* CNiuNiuRoom::doCreateRoomPlayerObject()
{
	return new IRoomPlayer();
}

uint8_t CNiuNiuRoom::getMaxRate()
{
	return getReateByNiNiuType(CNiuNiuPeerCard::NiuNiuType::Niu_FiveSmall,10);
}

uint32_t CNiuNiuRoom::getBaseBet()
{
	return 20 ;
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

	uint8_t nSeatCnt = getSeatCount() ;
	for ( uint8_t nIdx = 0; nIdx < nSeatCnt; ++nIdx )
	{
		ISitableRoomPlayer* pp = getPlayerByIdx(nIdx) ;
		if ( pp )
		{
			pp->onGameBegin();
		}
	}
	getPoker()->RestAllPoker();
	CLogMgr::SharedLogMgr()->PrintLog("room game begin");
}

void CNiuNiuRoom::onGameDidEnd()
{
	stMsgSaveLog msgSaveLog ;
	msgSaveLog.nLogType = eLog_NiuNiuGameResult ;
	msgSaveLog.nTargetID = getRoomID() ;
	msgSaveLog.nJsonExtnerLen = 0 ;
	msgSaveLog.vArg[0] = getPlayerByIdx(m_nBankerIdx)->getUserUID();
	msgSaveLog.vArg[1] = m_nBetBottomTimes ;
	msgSaveLog.vArg[2] = getBaseBet() * m_nBetBottomTimes ;
	m_arrPlayers.clear() ;

	ISitableRoom::onGameDidEnd();
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
			refPlayer["offset"] = pNiuPlayer->getCoinOffsetThisGame() ;
			refPlayer["coin"] = (int32_t)pNiuPlayer->getCoin() ;
			m_arrPlayers[pNiuPlayer->getIdx()] = refPlayer ;
		}

		pSitDown->removeState(eRoomPeer_CanAct);

		if ( pSitDown->isHaveState(eRoomPeer_WillLeave) )
		{
			CLogMgr::SharedLogMgr()->PrintLog("game end ,player uid = %d should leave ",pSitDown->getUserUID()) ;
			doPlayerLeave(pSitDown->getSessionID()) ;
		}
		else if ( pSitDown->isHaveState(eRoomPeer_StandUp) )
		{
			CLogMgr::SharedLogMgr()->PrintLog(" game end player uid = %d should stand up  ",pSitDown->getUserUID()) ;
			playerStandUp(pSitDown) ;
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

	CLogMgr::SharedLogMgr()->PrintLog("room game End");
}

void CNiuNiuRoom::doPlayerLeave(uint32_t nPlayerSessionID )
{
	IRoomPlayer* pp = getPlayerBySessionID(nPlayerSessionID) ;
	if ( pp == nullptr )
	{
		//assert(0&& "not in room how leave");
		CLogMgr::SharedLogMgr()->ErrorLog("you are not in room how to leave") ;
		return ;
	}

	ISitableRoomPlayer* pSitDown = getSitdownPlayerBySessionID(nPlayerSessionID);

	stMsgCrossServerRequest msgEnter ;
	msgEnter.cSysIdentifer = ID_MSG_PORT_DATA ;
	msgEnter.nJsonsLen = 0 ;
	msgEnter.nReqOrigID = getRoomID();
	msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
	msgEnter.nRequestType = eCrossSvrReq_LeaveRoomRet ;
	msgEnter.nTargetID = pp->getUserUID() ;
	msgEnter.vArg[0] = eRoom_NiuNiu ;
	msgEnter.vArg[1] = getRoomID() ;
	if ( pSitDown )
	{
		if (pSitDown->isHaveState(eRoomPeer_CanAct) == false )
		{
			playerStandUp(pSitDown);

			removePlayer(pp);
			sendMsgToPlayer(&msgEnter,sizeof(msgEnter),getRoomID()) ;
			CLogMgr::SharedLogMgr()->PrintLog("sitdown player , uid = %d leave room",pp->getUserUID() );
		}
		else
		{

			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d leave room but some error ,state = %d, player state = %d",pSitDown->getUserUID(),getCurRoomState()->getStateID(),pSitDown->getState()) ;
			pSitDown->setState(eRoomPeer_WillLeave) ;
		}
	}
	else
	{
		sendMsgToPlayer(&msgEnter,sizeof(msgEnter),getRoomID()) ;
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d leave room",pp->getUserUID() );
		pp->willLeave();
		removePlayer(pp) ;
	}
}