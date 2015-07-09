#include "TaxasPokerScene.h"
#include "LocalTaxasPlayer.h"
#include "ClientApp.h"
#include "TaxasMessageDefine.h"
#include "TaxasPokerSceneState.h"
Scene* CTaxasPokerScene::createScene()
{
	auto scn = Scene::create();
	CTaxasPokerScene* pLayer = CTaxasPokerScene::create();
	scn->addChild(pLayer);
	return scn ;
}

bool CTaxasPokerScene::init()
{
	IBaseScene::init();
	memset(m_vAllState,0,sizeof(m_vAllState));
	m_eCurState = eRoomState_TP_MAX ;
	m_ptMailPoolWorldPt = Vec2::ZERO;
	memset(&m_tGameData,0,sizeof(m_tGameData));
	Node* pRoot = CSLoader::getInstance()->createNodeWithFlatBuffersFile("res/Game.csb");
	addChild(pRoot);
	auto cSize = pRoot->getContentSize();
	pRoot->setContentSize(Director::getInstance()->getWinSize()) ;
	ui::Helper::doLayout(pRoot);

	m_pMainPool = (Label*)pRoot->getChildByName("mainPool");

	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD ; ++nIdx )
	{
		m_vPublicCard[nIdx] = (Sprite*)pRoot->getChildByName(String::createWithFormat("public%d",nIdx)->getCString());
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vVicePool[nIdx] = (Label*)pRoot->getChildByName(String::createWithFormat("vicePool%d",nIdx)->getCString());
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		Node* pLayerNode = (Sprite*)pRoot->getChildByName(String::createWithFormat("player%d",nIdx)->getCString());
		m_vTaxasPlayers[nIdx] = CTaxasPlayer::create(pLayerNode,nIdx,m_tGameData.getTaxasPlayerData(nIdx));
		addChild(m_vTaxasPlayers[nIdx]); // just for : when scene delte, the the object can be delete ; addchild here not for display or rending .
		m_vPosOfTaxasPlayers[nIdx] = pLayerNode->getPosition();
		m_vTaxasPlayers[nIdx]->setClickPhotoCallBack(CC_CALLBACK_1(CTaxasPokerScene::onClickPlayerPhoto,this));
	}

	Node* pLayerNode = (Sprite*)pRoot->getChildByName("self");
	m_pLocalPlayer = CLocalTaxasPlayer::create(pLayerNode,8,m_tGameData.getTaxasPlayerData(0));
	addChild(m_pLocalPlayer); // just for : when scene delte, the the object can be delete ; addchild here not for display or rending .
	refreshContent();
	
	goToState(eRoomState_TP_WaitJoin); 
	return true ;
}

bool CTaxasPokerScene::onMsg(stMsg* pmsg )
{
	bool b = IBaseScene::onMsg(pmsg);
	if ( b )return true ;

	m_tGameData.onMsg(pmsg);

	switch (pmsg->usMsgType)
	{
	case MSG_TP_ENTER_STATE:
		{
			stMsgTaxasRoomEnterState* pRet = (stMsgTaxasRoomEnterState*)pmsg ;
			goToState((eRoomState)pRet->nNewState) ;
		}
		break;
	case MSG_TP_START_ROUND:
		{
			goToState(eRoomState_TP_BetBlind) ;
		}
		break;
	case MSG_TP_PRIVATE_CARD:
		{
			goToState(eRoomState_TP_PrivateCard);
		}
		break;
	case MSG_TP_WAIT_PLAYER_ACT:
		{
			goToState(eRoomState_TP_Beting) ;
		}
		break;
	case MSG_TP_ONE_BET_ROUND_RESULT:
		{
			goToState(eRoomState_TP_OneRoundBetEndResult,pmsg);
		}
		break;
	case MSG_TP_GAME_RESULT:
		{
			goToState(eRoomState_TP_GameResult,pmsg) ;
		}
		break;
	case MSG_TP_PUBLIC_CARD:
		{
			goToState( eRoomState_TP_PublicCard ) ;
		}
		break;
	default:
		return false;
	}

	if ( m_vAllState[m_eCurState] )
	{
		m_vAllState[m_eCurState]->onMsg(pmsg);
	}

	return true ;
}

void CTaxasPokerScene::refreshContent()
{
	// refresh public card 
	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD; ++nIdx )
	{
		uint8_t nCardNum = m_tGameData.vPublicCardNums[nIdx];
		m_vPublicCard[nIdx]->setVisible(nCardNum !=0 ) ;
		if ( nCardNum != 0 )
		{
			m_vPublicCard[nIdx]->setSpriteFrame(CClientApp::getCardSpriteByCompsiteNum(nCardNum)) ;
		}
	}

	// refresh main pool
	m_pMainPool->setVisible(m_tGameData.nCurMainBetPool > 0 );
	if ( m_tGameData.nCurMainBetPool )
	{
		m_pMainPool->setString(String::createWithFormat("%I64d",m_tGameData.nCurMainBetPool)->getCString());
	}

	// refresh vice pool
	refreshVicePools();

	// refresh all players ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vTaxasPlayers[nIdx]->getRoot()->isVisible() )
		{
			m_vTaxasPlayers[nIdx]->refreshContent();
		}
	}
	
	if ( m_pLocalPlayer->getRoot()->isVisible() )
	{
		m_pLocalPlayer->refreshContent();
	}
}

void CTaxasPokerScene::refreshVicePools()
{
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vVicePool[nIdx]->setVisible(m_tGameData.vVicePool[nIdx] > 0 ) ;
		if ( m_tGameData.vVicePool[nIdx] > 0 )
		{ 
			m_vVicePool[nIdx]->setString(String::createWithFormat("%I64d",m_tGameData.vVicePool[nIdx])->getCString());
		}
	}
}

void CTaxasPokerScene::relayoutPlayerLocalPos( uint8_t nLocalPlayerSvrIdx )
{
	CTaxasPlayer* pPosPayer = getLocalPlayer();
	uint8_t nCurLocalIdx = pPosPayer->getLocalIdx();
	uint8_t nOffsetIdx = MAX_PEERS_IN_TAXAS_ROOM - 1 - nCurLocalIdx ; // all TaxasPlayer should move offsetIdx ;
	if ( nOffsetIdx == 0 )  // should not change any place and pos
	{
		return ;
	}
	
	// do relayout 
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		CTaxasPlayer* pPeer = m_vTaxasPlayers[nIdx];
		doLayoutTaxasPlayer(pPeer,nOffsetIdx);
	}
	doLayoutTaxasPlayer(m_pLocalPlayer,nOffsetIdx);
}

int8_t CTaxasPokerScene::getLocalIdxBySvrIdx( uint8_t nSvrIdx )
{
	assert(nSvrIdx < MAX_PEERS_IN_TAXAS_ROOM && "invalid svr idx" );

	return getTaxasPlayerBySvrIdx(nSvrIdx)->getLocalIdx();
}

CTaxasPlayer* CTaxasPokerScene::getTaxasPlayerBySvrIdx(uint8_t nSvrIdx )
{
	assert(nSvrIdx < MAX_PEERS_IN_TAXAS_ROOM && "invalid svr idx" );

	if ( nSvrIdx == getLocalPlayer()->getServerIdx() && getLocalPlayer()->getRoot()->isVisible() )
	{
		return getLocalPlayer();
	}

	for ( int8_t nidx = 0 ; nidx < MAX_PEERS_IN_TAXAS_ROOM ; ++nidx )
	{
		if ( m_vTaxasPlayers[nidx]->getServerIdx() == nSvrIdx )
		{
			return m_vTaxasPlayers[nidx];
		}
	}
	return NULL ;
}

void CTaxasPokerScene::doLayoutTaxasPlayer(CTaxasPlayer*pPlayer,uint8_t nOffsetIdx )
{
	if ( nOffsetIdx == 0 )
	{
		return ;
	}

	float fMoveTime = 0.01 ;
	uint8_t nCurIdx = pPlayer->getLocalIdx();
	uint8_t nTargetIdx = nCurIdx + nOffsetIdx ;

	// if not update , juset updat local idx, infact this will be alwasy true , you shoud not update local index outside the function ;
	if ( pPlayer->getLocalIdx() != nTargetIdx%MAX_PEERS_IN_TAXAS_ROOM )
	{
		pPlayer->setLocalIdx(nTargetIdx%MAX_PEERS_IN_TAXAS_ROOM) ;
	}
	else
	{
		// infact this will not invoke , because you should update player local index out this function 
		// here will juset do a check and try to fix ;
		nCurIdx = (nCurIdx + MAX_PEERS_IN_TAXAS_ROOM - nOffsetIdx  ) % MAX_PEERS_IN_TAXAS_ROOM ;
		pPlayer->setPos(m_vPosOfTaxasPlayers[nCurIdx]) ;
	}

	Vector<FiniteTimeAction*> vAllMove ;
	for ( uint8_t nMoveIdx = nCurIdx + 1 ; nMoveIdx <= nTargetIdx ; ++nTargetIdx )
	{
		MoveTo* pto = MoveTo::create(fMoveTime,m_vPosOfTaxasPlayers[nMoveIdx%MAX_PEERS_IN_TAXAS_ROOM] ) ;
		vAllMove.pushBack(pto);
	}

	if ( nOffsetIdx == 1 )
	{
		pPlayer->getRoot()->runAction(vAllMove.at(0)) ;
	}
	else
	{
		Sequence* pseq = Sequence::create(vAllMove) ;
		pPlayer->getRoot()->runAction(pseq) ;
	}
}

void CTaxasPokerScene::distributePrivateCard()
{
	// distry delay ;
	Vec2 ptOrig (Director::getInstance()->getWinSize().width * 0.5,Director::getInstance()->getWinSize().height);
	float fDelay = 0 ;
	for ( uint8_t nCardIdx = 0 ; nCardIdx < TAXAS_PEER_CARD ; ++nCardIdx )
	{
		for ( uint8_t nIdx = m_tGameData.nBankerIdx ; nIdx < MAX_PEERS_IN_TAXAS_ROOM * 2; ++nIdx )
		{
			uint8_t nRealIdx = nIdx % MAX_PEERS_IN_TAXAS_ROOM ;
			CTaxasPlayer* pPlayer = getTaxasPlayerBySvrIdx(nRealIdx);
			if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
			{
				pPlayer->distributeHoldCard(ptOrig,nCardIdx,TIME_TAXAS_ONE_CARD_DISTRIBUTE,fDelay);
				fDelay += TIME_TAXAS_DISTRIBUTE_HOLD_CARD_DELAY;
			}
		}
	}
}

void CTaxasPokerScene::playersBetCoinGoMainPool()
{
	bool bHave = false;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vTaxasPlayers[nIdx]->getRoot()->isVisible() && m_vTaxasPlayers[nIdx]->isHavePlayer() )
		{
			if ( m_vTaxasPlayers[nIdx]->betCoinGoToMainPool(getMainPoolWorldPos(),TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL) )
			{
				bHave = true;
			}
		}
	}

	if ( m_pLocalPlayer->getRoot()->isVisible() )
	{
		if ( m_pLocalPlayer->betCoinGoToMainPool(getMainPoolWorldPos(),TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL) )
		{
			bHave = true ;
		}
	}

	if ( bHave )
	{
		Director::getInstance()->getScheduler()->schedule([=](float ft){ onPlayersBetCoinArrived(0);},this,TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL,0,TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL,false,"PGM");
	}
}

void CTaxasPokerScene::onPlayersBetCoinArrived( uint8_t nNewVicePoolCnt )
{
	// refresh main pool and build vice pool ;
	// refresh main pool
	m_pMainPool->setVisible(m_tGameData.nCurMainBetPool > 0 );
	if ( m_tGameData.nCurMainBetPool )
	{
		m_pMainPool->setString(String::createWithFormat("%I64d",m_tGameData.nCurMainBetPool)->getCString());
	}

	// refresh vice pool
	refreshVicePools();
}

void CTaxasPokerScene::onPlayerGiveupCoinArrived()
{
	// refresh main pool ;
	m_pMainPool->setVisible(m_tGameData.nCurMainBetPool > 0 );
	if ( m_tGameData.nCurMainBetPool )
	{
		m_pMainPool->setString(String::createWithFormat("%I64d",m_tGameData.nCurMainBetPool)->getCString());
	}
}

Vec2 CTaxasPokerScene::getMainPoolWorldPos()
{
	if ( m_ptMailPoolWorldPt == Vec2::ZERO )
	{
		m_ptMailPoolWorldPt = m_pMainPool->getParent()->convertToWorldSpace(m_pMainPool->getPosition());
	}
	return m_ptMailPoolWorldPt ;
}

void CTaxasPokerScene::distributePublicCard(uint8_t nRound )
{
	uint8_t nidx = 0 ;
	uint8_t nCnt = 0 ;
	if ( nRound == 1 )
	{
		nidx = 0 ;
		nCnt = 3 ;
	}
	else if ( 2 == nRound )
	{
		 nidx = 3;
		 nCnt = 1 ;
	}
	else if ( 3 == nRound )
	{
		nidx = 4 ;
		nCnt = 1 ; 
	}

	for ( uint8_t n = nidx ; n < (nidx + nCnt); ++n )
	{
		m_vPublicCard[n]->setVisible(true);
		m_vPublicCard[n]->setSpriteFrame(CClientApp::getCardSpriteByCompsiteNum(getPokerData()->vPublicCardNums[n]));
	}
}

void CTaxasPokerScene::showAllPlayersFinalCard()
{
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vTaxasPlayers[nIdx]->getRoot()->isVisible() && m_vTaxasPlayers[nIdx]->isHavePlayer() )
		{
			bool bW = (m_vTaxasPlayers[nIdx]->getPlayerData().nStateFlag & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate ;
			if ( bW )
			{
				m_vTaxasPlayers[nIdx]->showFinalCard();
			}
		}
	}

	if ( m_pLocalPlayer->getRoot()->isVisible())
	{
		bool bW = (m_pLocalPlayer->getPlayerData().nStateFlag & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate ;
		if ( bW )
		{
			m_pLocalPlayer->showFinalCard();
		}
	}
}

void CTaxasPokerScene::winCoinGoToWinners(uint8_t nPoolIdx,uint64_t nCoinPerWinner,uint8_t vWinnerIdx[MAX_PEERS_IN_TAXAS_ROOM],uint8_t nWinnerCnt)
{
	if ( nPoolIdx >= MAX_PEERS_IN_TAXAS_ROOM  || nWinnerCnt >= MAX_PEERS_IN_TAXAS_ROOM )
	{
		CCLOG("invalid win pool idx = %d cnt = %d",nPoolIdx,nWinnerCnt);
		return ;
	}

	Vec2 pt = m_vVicePool[nPoolIdx]->getParent()->convertToWorldSpace(m_vVicePool[nPoolIdx]->getPosition());
	for ( uint8_t nidx = 0 ; nidx < nWinnerCnt ; ++nidx )
	{
		CTaxasPlayer* player = getTaxasPlayerBySvrIdx(vWinnerIdx[nidx]) ;
		if (player && player->isHavePlayer() && player->isHaveState(eRoomPeer_WaitCaculate) )
		{
			player->winCoinGoToPlayer(pt,TIME_TAXAS_CACULATE_PER_BET_POOL );
		}
		else
		{
			CCLOG("win player state error idx = %d ",vWinnerIdx[nidx]);
		}
	}
}

void CTaxasPokerScene::goToState(eRoomState eState,stMsg* pmsg )
{
	if ( eState == m_eCurState || eState >= eRoomState_TP_MAX )
	{
		return ;
	}

	if ( m_eCurState < eRoomState_TP_MAX && m_vAllState[m_eCurState] )
	{
		m_vAllState[m_eCurState]->leaveState();
	}

	m_eCurState = eState ;
	if ( m_vAllState[m_eCurState] )
	{
		m_vAllState[m_eCurState]->enterState(pmsg);
		return ;
	}

	switch ( m_eCurState )
	{
	case eRoomState_TP_WaitJoin:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerWaitJoinState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	case eRoomState_TP_BetBlind:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerBlindBetState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	case eRoomState_TP_PrivateCard:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerPrivateCardState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	case eRoomState_TP_Beting:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerBettingState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	case eRoomState_TP_OneRoundBetEndResult:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerOneBetRoundEndResultState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	case eRoomState_TP_PublicCard:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerPublicCardState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	case eRoomState_TP_GameResult:
		{
			m_vAllState[m_eCurState] = new CTaxasPokerGameResultState ;
			m_vAllState[m_eCurState]->init(this);
			addChild(m_vAllState[m_eCurState]);
			m_vAllState[m_eCurState]->release();
		}
		break;
	default:
		{
			CCLOG("error unknown state = %d",m_eCurState);
			m_eCurState = eRoomState_TP_WaitJoin ;
		}
		break;
	}

	if ( m_vAllState[m_eCurState] )
	{
		m_vAllState[m_eCurState]->enterState(pmsg);
	}
}

void CTaxasPokerScene::onClickPlayerPhoto(CTaxasPlayer*pPlayer)
{
	if ( pPlayer->isHavePlayer() )
	{
		CCLOG("request player detail uid = %d",pPlayer->getPlayerData().nUserUID);
	}
	else
	{
		// send msg to sit down 
		stMsgTaxasPlayerSitDown msg ;
		msg.nRoomID = getPokerData()->nRoomID;
		msg.nSeatIdx = pPlayer->getServerIdx();
		msg.nTakeInMoney = getPokerData()->nMiniTakeIn ;
		sendMsg(&msg,sizeof(msg));
		CCLOG("send msg to sit down ");
	}
}