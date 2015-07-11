#include "TaxasPokerSceneState.h"
#include "TaxasMessageDefine.h"
#include "TaxasPokerScene.h"
#include "TaxasPokerData.h"
#include "LocalTaxasPlayer.h"
#include "CommonData.h"
#include "ClientApp.h"
// base scene
bool CTaxasPokerSceneStateBase::init(CTaxasPokerScene* pScene)
{ 
	m_eState = eRoomState_TP_MAX ; 
	cocos2d::Node::init();
	m_pScene = pScene ;
	return true ;
}

bool CTaxasPokerSceneStateBase::onMsg(stMsg* pmsg )
{
	if (pmsg == nullptr )
	{
		return true ;
	}

	switch ( pmsg->usMsgType )
	{
	case MSG_TP_ROOM_BASE_INFO:
		{
			m_pScene->onRecievedRoomInfo();
		}
		break;
	case MSG_TP_ROOM_VICE_POOL:
		{
			m_pScene->refreshVicePools();
		}
		break;
	case MSG_TP_ROOM_STAND_UP:
		{
			stMsgTaxasRoomStandUp* pRet = (stMsgTaxasRoomStandUp*)pmsg ;
			if ( m_pScene->getLocalPlayer()->getServerIdx() == pRet->nSeatIdx )
			{
				m_pScene->getLocalPlayer()->getRoot()->setVisible(false);
				m_pScene->getLocalPlayer()->setServerIdx(MAX_PEERS_IN_TAXAS_ROOM) ;
			}
			m_pScene->getTaxasPlayerBySvrIdx(pRet->nSeatIdx)->getRoot()->setVisible(true);
			m_pScene->getTaxasPlayerBySvrIdx(pRet->nSeatIdx)->refreshContent();
		}
		break;
	case MSG_TP_PLAYER_SIT_DOWN:
		{
			stMsgTaxasPlayerSitDownRet* pRet = (stMsgTaxasPlayerSitDownRet*)pmsg ;
			const char* pErrorS = "unknown error sit down " ;
			switch ( pRet->nRet )
			{
			case 0:
				return true ;
			case 1:
				{
					pErrorS = "pos have peer" ;
				}
				break;
			case 2:
				{
					pErrorS = "invalid session id, not in room";
				}
				break;
			case 3:
				{
					pErrorS = "invalid take in coin";
				}
			case 4:
				{
					pErrorS = "money not engouth";
				}
				break;
			default:
				break;
			}
			cocos2d::MessageBox(pErrorS,"sit down error");
		}
		break;
	case MSG_TP_PLAYER_ACT:
		{
			stMsgTaxasPlayerActRet* pRet = (stMsgTaxasPlayerActRet*)pmsg ;
			const char* pErrorS = "unknown error sit down " ;
			switch (pRet->nRet)
			{
			case 0:
				{
					return true ;
				}
				break;
			case 1:
				{
					pErrorS = "not your turn";
				}
				break;
			case 2 :
				pErrorS = "you are not in this game";
				break;
			case 3:
				pErrorS = "state error , you can not act";
				break;
			case 4:
				pErrorS = "unknown act type";
				break;
			case 5:
				pErrorS = "can not do this act" ;
				break;
			case 6:
				pErrorS = "coin not engough" ;
				break;
			default:
				break;
			}
			cocos2d::MessageBox(pErrorS,"sit down error");
		}
		break;
	case MSG_TP_ROOM_PLAYER_DATA:
	case MSG_TP_ROOM_SIT_DOWN:
		{
			stTaxasPeerBaseData* ttPlayerData = nullptr ;
			if ( MSG_TP_ROOM_SIT_DOWN == pmsg->usMsgType )
			{
				stMsgTaxasRoomSitDown* pRet = (stMsgTaxasRoomSitDown*)pmsg ;
				ttPlayerData = &pRet->tPlayerData;
			}
			else
			{
				stMsgTaxasRoomInfoPlayerData* pRet = (stMsgTaxasRoomInfoPlayerData*)pmsg;
				ttPlayerData = &pRet->tPlayerData;
			}
			
			bool bLocalUser = CClientApp::getInstance()->getPlayerData()->nUserUID == ttPlayerData->nUserUID ;
			CTaxasPlayer* pPosPlayer = m_pScene->getTaxasPlayerBySvrIdx(ttPlayerData->nSeatIdx);
			if ( bLocalUser )
			{
				CCLOG("my svr idx = %d",ttPlayerData->nSeatIdx);
				// copy pPosPlayer's info to m_LocalPlayer, because m_LocalPlayer will logic work on pPosPlayer ;
				m_pScene->getLocalPlayer()->getRoot()->setVisible(true);
				m_pScene->getLocalPlayer()->setServerIdx(ttPlayerData->nSeatIdx) ;
				m_pScene->getLocalPlayer()->setLocalIdx(pPosPlayer->getLocalIdx());
				m_pScene->getLocalPlayer()->getRoot()->setPosition(pPosPlayer->getRoot()->getPosition());
				m_pScene->getLocalPlayer()->bindPlayerData(m_pScene->getPokerData()->getTaxasPlayerData(ttPlayerData->nSeatIdx)) ;
				pPosPlayer->setVisible(false) ;
				m_pScene->relayoutPlayerLocalPos(ttPlayerData->nSeatIdx);
			}
			m_pScene->getTaxasPlayerBySvrIdx(ttPlayerData->nSeatIdx)->refreshContent();
		}
		break;
	default:
		return false;
	}
	return true ;
}

// wait join state
bool CTaxasPokerWaitJoinState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_WaitJoin ;
	return true ;
}

bool CTaxasPokerWaitJoinState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}
	return false ;
}

void CTaxasPokerWaitJoinState::enterState(stMsg* pmsg) 
{
	CTaxasPokerSceneStateBase::enterState(pmsg);
	m_pScene->getPokerData()->resetRuntimeData();
	m_pScene->refreshContent();
}

// blind bet state
bool CTaxasPokerBlindBetState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_BetBlind;
	return true ;
}

void CTaxasPokerBlindBetState::enterState(stMsg* pmsg)
{
	CTaxasPokerSceneStateBase::enterState(pmsg);
}

bool CTaxasPokerBlindBetState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}

	if (MSG_TP_START_ROUND == pmsg->usMsgType )
	{
		 stMsgTaxasRoomStartRound* pRet = (stMsgTaxasRoomStartRound*)pmsg ;
		 CTaxasPlayer* player = m_pScene->getTaxasPlayerBySvrIdx(pRet->nLittleBlindIdx);
		 player->betBlind(m_pScene->getPokerData()->nLittleBlind);
		 player = m_pScene->getTaxasPlayerBySvrIdx(pRet->nBigBlindIdx);
		 player->betBlind(m_pScene->getPokerData()->nLittleBlind * 2 );
		 CCLOG("start round");
	}
	return false ;
}

// private card
bool CTaxasPokerPrivateCardState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_PrivateCard;
	return true ;
}

void CTaxasPokerPrivateCardState::enterState(stMsg* pmsg)
{
	CTaxasPokerSceneStateBase::enterState(pmsg);
}

bool CTaxasPokerPrivateCardState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}

	if ( MSG_TP_PRIVATE_CARD == pmsg->usMsgType )
	{
		m_pScene->distributePrivateCard();
	}

	return false ;
}

// beting state
bool CTaxasPokerBettingState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_Beting;
	return true ;
}

void CTaxasPokerBettingState::enterState(stMsg* pmsg) 
{
	CTaxasPokerSceneStateBase::enterState(pmsg) ;
}

bool CTaxasPokerBettingState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}

	switch ( pmsg->usMsgType )
	{
	case MSG_TP_WAIT_PLAYER_ACT:
		{
			stMsgTaxasRoomWaitPlayerAct* pRet = (stMsgTaxasRoomWaitPlayerAct*)pmsg ;
			if ( pRet->nActPlayerIdx >= MAX_PEERS_IN_TAXAS_ROOM )
			{
				CCLOG("invalid act player idx = %d\n",pRet->nActPlayerIdx);
				return true ;
			}
			CCLOG("wait act idx = %d ",pRet->nActPlayerIdx);
			m_pScene->getTaxasPlayerBySvrIdx(pRet->nActPlayerIdx)->onWaitAction();
		}
		break;
	case MSG_TP_ROOM_ACT:
		{
			stMsgTaxasRoomAct* pret = (stMsgTaxasRoomAct*)pmsg;
			if ( pret->nPlayerIdx >= MAX_PEERS_IN_TAXAS_ROOM )
			{
				CCLOG("MSG_TP_ROOM_ACT invalid act player idx = %d\n",pret->nPlayerIdx);
				return true ;
			}
			CTaxasPlayer* pPlayer = m_pScene->getTaxasPlayerBySvrIdx(pret->nPlayerIdx);
			pPlayer->onAct(pret->nPlayerAct,pret->nValue);
			if ( pret->nPlayerAct == eRoomPeerAction_GiveUp )
			{
				pPlayer->betCoinGoToMainPool(m_pScene->getMainPoolWorldPos(),TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL);
				Director::getInstance()->getScheduler()->schedule([=](float ft){ m_pScene->onPlayerGiveupCoinArrived();},this,TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL,0,TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL,false,"giveUpCoin");
			}
			//pPlayer->refreshContent();
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CTaxasPokerBettingState::onLocalPlayerActCallBack(CTaxasPlayer* pPlayer ,uint8_t nActValue, uint32_t nValue )
{
	if ( pPlayer->isHavePlayer() == false )
	{
		assert(0&&"empty player can act ?");
		return ;
	}

	if ( nActValue >= eRoomPeerAction_Max )
	{
		assert(0&&"error act value");
		return ;
	}

	if ( m_pScene->getLocalPlayer()->getServerIdx() != m_pScene->getPokerData()->nCurWaitPlayerActionIdx && "not my turn" )
	{
		assert(0 && "not my turn");
		return ;
	}

	stMsgTaxasPlayerAct msgsend ;
	msgsend.nPlayerAct = nActValue ;
	msgsend.nRoomID = m_pScene->getPokerData()->nRoomID ;
	msgsend.nValue = nValue ;
	// value check
	if ( nActValue == eRoomPeerAction_Add )
	{
		uint32_t nlowLimit = m_pScene->getPokerData()->getPlayerAddCoinLowLimit(pPlayer->getServerIdx());
		uint64_t nupLimit = m_pScene->getPokerData()->getPlayerAddCoinUpLimit(pPlayer->getServerIdx());
		if ( msgsend.nValue > nupLimit )
		{
			msgsend.nValue = nupLimit ;
		}

		if ( msgsend.nValue < nlowLimit )
		{
			msgsend.nValue = nlowLimit ;
		}

		if ( msgsend.nValue > m_pScene->getPokerData()->getTaxasPlayerData(pPlayer->getServerIdx())->nTakeInMoney )
		{
			msgsend.nPlayerAct = eRoomPeerAction_AllIn ;
			msgsend.nValue = m_pScene->getPokerData()->getTaxasPlayerData(pPlayer->getServerIdx())->nTakeInMoney ;
		}
	}
	else if ( eRoomPeerAction_Follow == nActValue )
	{
		uint64_t nBet = m_pScene->getPokerData()->nMostBetCoinThisRound - m_pScene->getPokerData()->getTaxasPlayerData(pPlayer->getServerIdx())->nBetCoinThisRound ;
		if ( nBet >= m_pScene->getPokerData()->getTaxasPlayerData(pPlayer->getServerIdx())->nTakeInMoney )
		{
			msgsend.nPlayerAct = eRoomPeerAction_AllIn ;
			msgsend.nValue = m_pScene->getPokerData()->getTaxasPlayerData(pPlayer->getServerIdx())->nTakeInMoney ;
		}
	}

	m_pScene->sendMsg(&msgsend,sizeof(msgsend)) ;
}

// one bet round end
bool CTaxasPokerOneBetRoundEndResultState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_OneRoundBetEndResult ;
	return true ;
}

void CTaxasPokerOneBetRoundEndResultState::enterState(stMsg* pmsg)
{
	CTaxasPokerSceneStateBase::enterState(pmsg);
	if ( m_pScene->getLocalPlayer()->getRoot()->isVisible() )
	{
		m_pScene->getLocalPlayer()->hideActBtns() ;
	}
}

bool CTaxasPokerOneBetRoundEndResultState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}

	if (pmsg->usMsgType == MSG_TP_ONE_BET_ROUND_RESULT )
	{
		stMsgTaxasRoomOneBetRoundResult* pRet = (stMsgTaxasRoomOneBetRoundResult*)pmsg;

		// all peer coin go to main pool 
		uint8_t nNewVicePoolCnt = pRet->nNewVicePoolCnt;
		m_pScene->playersBetCoinGoMainPool();
	}

	return false ;
}

// public card state
bool CTaxasPokerPublicCardState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_PublicCard ;
	return true ;
}


void CTaxasPokerPublicCardState::enterState(stMsg* pmsg)
{
	CTaxasPokerSceneStateBase::enterState(pmsg);
}

void CTaxasPokerPublicCardState::leaveState()
{
	CTaxasPokerSceneStateBase::leaveState();
}

bool CTaxasPokerPublicCardState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}

	if ( MSG_TP_PUBLIC_CARD == pmsg->usMsgType )
	{
		stMsgTaxasRoomPublicCard* pRet = (stMsgTaxasRoomPublicCard*)pmsg ;
		m_pScene->distributePublicCard(pRet->nCardSeri );
		CCLOG("distributePublicCard(nPublicRound)");
	}
	return false ;
}

// game result state
bool CTaxasPokerGameResultState::init(CTaxasPokerScene* pScene)
{
	CTaxasPokerSceneStateBase::init(pScene);
	m_eState = eRoomState_TP_GameResult ;
	return true ;
}

void CTaxasPokerGameResultState::enterState(stMsg* pmsg)
{
	CTaxasPokerSceneStateBase::enterState(pmsg);
	m_pScene->showAllPlayersFinalCard();
	m_vAllResult.clear();
	m_bCoinFlying = false ;
}

bool CTaxasPokerGameResultState::onMsg(stMsg* pmsg )
{
	if ( CTaxasPokerSceneStateBase::onMsg(pmsg) )
	{
		return true ;
	}

	if ( pmsg == nullptr || pmsg->usMsgType != MSG_TP_GAME_RESULT  )
	{
		return false;
	}

	stMsgTaxasRoomGameResult* pRet = (stMsgTaxasRoomGameResult*)pmsg;
	if ( m_bCoinFlying )
	{
		m_vAllResult.push_back(*pRet);
	}
	else
	{
		m_bCoinFlying = true ;
		m_pScene->winCoinGoToWinners(pRet->nPoolIdx,pRet->nCoinPerWinner,pRet->vWinnerIdx,pRet->nWinnerCnt);
		if ( pRet->bIsLastOne == false )
		{
			Director::getInstance()->getScheduler()->schedule([this](float f ){
				if ( m_vAllResult.empty() )
				{
					m_bCoinFlying = false ;
					Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
					return ;
				}

				stMsgTaxasRoomGameResult& pRet = m_vAllResult.front();
				bool bLast = pRet.bIsLastOne ;
				m_pScene->winCoinGoToWinners(pRet.nPoolIdx,pRet.nCoinPerWinner,pRet.vWinnerIdx,pRet.nWinnerCnt);
				m_vAllResult.erase(m_vAllResult.begin()) ;
				if ( bLast )
				{
					Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
				}

			},this,TIME_TAXAS_WIN_COIN_GOTO_PLAYER,MAX_PEERS_IN_TAXAS_ROOM,TIME_TAXAS_WIN_COIN_GOTO_PLAYER,false,"winResult");
		}
	}
	return true ;
}

void CTaxasPokerGameResultState::leaveState()
{
	CTaxasPokerSceneStateBase::leaveState();
	m_pScene->getPokerData()->resetRuntimeData();
	m_pScene->refreshContent();
	m_vAllResult.clear();
	m_bCoinFlying = false ;
}