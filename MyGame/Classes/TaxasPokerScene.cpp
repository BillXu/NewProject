#include "TaxasPokerScene.h"
#include "LocalTaxasPlayer.h"
#include "ClientApp.h"
#include "TaxasMessageDefine.h"
#include "TaxasPokerSceneState.h"
#include "chatLayer.h"
#include "GotyeAPI.h"
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

	m_pMainPool = (ui::Text*)pRoot->getChildByName("mainPool");

	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD ; ++nIdx )
	{
		m_vPublicCard[nIdx] = (Sprite*)pRoot->getChildByName(String::createWithFormat("public%d",nIdx)->getCString());
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vVicePool[nIdx] = (ui::Text*)pRoot->getChildByName(String::createWithFormat("vicePool%d",nIdx)->getCString());
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		Node* pLayerNode = (Sprite*)pRoot->getChildByName(String::createWithFormat("player%d",nIdx)->getCString());
		m_vTaxasPlayers[nIdx] = CTaxasPlayer::create(pLayerNode,nIdx,m_tGameData.getTaxasPlayerData(nIdx));
		addChild(m_vTaxasPlayers[nIdx]); // just for : when scene delte, the the object can be delete ; addchild here not for display or rending .
		m_vPosOfTaxasPlayers[nIdx] = pLayerNode->getPosition();
		m_vTaxasPlayers[nIdx]->setClickPhotoCallBack(CC_CALLBACK_1(CTaxasPokerScene::onClickPlayerPhoto,this));
		m_vTaxasPlayers[nIdx]->createCoinAni(pRoot);
	}

	Node* pLayerNode = (Sprite*)pRoot->getChildByName("self");
	m_pLocalPlayer = CLocalTaxasPlayer::create(pLayerNode,8,m_tGameData.getTaxasPlayerData(0));
	m_pLocalPlayer->setClickPhotoCallBack(CC_CALLBACK_1(CTaxasPokerScene::onClickPlayerPhoto,this));
	addChild(m_pLocalPlayer); // just for : when scene delte, the the object can be delete ; addchild here not for display or rending .
	m_pLocalPlayer->getRoot()->setVisible(false);
	m_pLocalPlayer->createCoinAni(pRoot);
	refreshContent();

	m_pChatLayer = CChatLayer::create();
	addChild(m_pChatLayer);
	m_pChatLayer->setCallBack(CC_CALLBACK_2(CTaxasPokerScene::chatInputCallBack,this));
	
	goToState(eRoomState_TP_WaitJoin); 
	return true ;
}

bool CTaxasPokerScene::onMsg(stMsg* pmsg )
{
	bool b = IBaseScene::onMsg(pmsg);
	if ( b )return true ;
	//CCLOG("msg id = %d",pmsg->usMsgType);

	m_tGameData.onMsg(pmsg);

	switch (pmsg->usMsgType)
	{
	case MSG_TP_ENTER_STATE:
		{
			stMsgTaxasRoomEnterState* pRet = (stMsgTaxasRoomEnterState*)pmsg ;
			goToState((eRoomState)pRet->nNewState) ;
			return true;
		}
		break;
	//case MSG_TP_START_ROUND:
	//	{
	//		goToState(eRoomState_TP_BetBlind) ;
	//	}
	//	break;
	//case MSG_TP_PRIVATE_CARD:
	//	{
	//		goToState(eRoomState_TP_PrivateCard);
	//	}
	//	break;
	//case MSG_TP_WAIT_PLAYER_ACT:
	//	{
	//		goToState(eRoomState_TP_Beting) ;
	//	}
	//	break;
	//case MSG_TP_ONE_BET_ROUND_RESULT:
	//	{
	//		goToState(eRoomState_TP_OneRoundBetEndResult,pmsg);
	//	}
	//	break;
	//case MSG_TP_GAME_RESULT:
	//	{
	//		goToState(eRoomState_TP_GameResult,pmsg) ;
	//	}
	//	break;
	//case MSG_TP_PUBLIC_CARD:
	//	{
	//		goToState( eRoomState_TP_PublicCard ) ;
	//	}
	//	break;
	default:
		break;
	}

	if ( m_vAllState[m_eCurState] )
	{
		m_vAllState[m_eCurState]->onMsg(pmsg);
	}

	return true ;
}

void CTaxasPokerScene::onRecievedRoomInfo()
{
	CCLOG("temp code here set for chat room id");
	getPokerData()->nChatRoomID = 14361018 ;
	enterChatRoom();
	refreshContent();
	// goto target 
	goToState((eRoomState)getPokerData()->eCurRoomState );
}

void CTaxasPokerScene::enterChatRoom()
{
	//GotyeRoom room (getPokerData()->nChatRoomID);
	//if ( GotyeAPI::getInstance()->isInRoom(room) == false )
	//{
	//	GotyeAPI::getInstance()->enterRoom(room);
	//}
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
			m_vTaxasPlayers[nIdx]->refreshContent(this);
		}
	}
	
	if ( m_pLocalPlayer->getRoot()->isVisible() )
	{
		m_pLocalPlayer->refreshContent(this);
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

	float fMoveTime = 0.05 ;
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
		nTargetIdx = nCurIdx + nOffsetIdx ;
		pPlayer->setPos(m_vPosOfTaxasPlayers[nCurIdx]) ;
	}

	Vector<FiniteTimeAction*> vAllMove ;
	for ( uint8_t nMoveIdx = nCurIdx + 1 ; nMoveIdx <= nTargetIdx ; ++nMoveIdx )
	{
		uint8_t nRIdx = nMoveIdx%MAX_PEERS_IN_TAXAS_ROOM;
		MoveTo* pto = MoveTo::create(fMoveTime,m_vPosOfTaxasPlayers[nRIdx] ) ;
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
		for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
		{
			uint8_t nRealIdx = ( m_tGameData.nBankerIdx +  nIdx ) % MAX_PEERS_IN_TAXAS_ROOM ;
			CTaxasPlayer* pPlayer = getTaxasPlayerBySvrIdx(nRealIdx);
			if ( pPlayer->isHaveState(eRoomPeer_CanAct) )
			{
				pPlayer->distributeHoldCard(ptOrig,nCardIdx,TIME_TAXAS_DISTRIBUTE_ONE_HOLD_CARD,fDelay);
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
		if ( m_vTaxasPlayers[nIdx]->getRoot()->isVisible() && m_vTaxasPlayers[nIdx]->isHavePlayer() && m_vTaxasPlayers[nIdx]->getPlayerData().nBetCoinThisRound > 0 )
		{
			if ( m_vTaxasPlayers[nIdx]->betCoinGoToMainPool(getMainPoolWorldPos(),TIME_TAXAS_WAIT_COIN_GOTO_MAIN_POOL) )
			{
				bHave = true;
			}
		}
	}

	if ( m_pLocalPlayer->getRoot()->isVisible() && m_pLocalPlayer->getPlayerData().nBetCoinThisRound > 0 )
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
	CCLOG("onPlayersBetCoinArrived");
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
	if ( nRound == 0 )
	{
		nidx = 0 ;
		nCnt = 3 ;
	}
	else if ( 1 == nRound )
	{
		 nidx = 3;
		 nCnt = 1 ;
	}
	else if ( 2 == nRound )
	{
		nidx = 4 ;
		nCnt = 1 ; 
	}
	else
	{
		CCLOG("invalid round value = %d",nRound);
		return ;
	}

	for ( uint8_t n = nidx ; n < (nidx + nCnt); ++n )
	{
		m_vPublicCard[n]->setVisible(true);
		m_vPublicCard[n]->setSpriteFrame(CClientApp::getCardSpriteByCompsiteNum(getPokerData()->vPublicCardNums[n]));
	}
}

void CTaxasPokerScene::showAllPlayersFinalCard()
{
	if ( getPlayerCntWithState(eRoomPeer_WaitCaculate) < 2 )
	{
		return ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vTaxasPlayers[nIdx]->getRoot()->isVisible() && m_vTaxasPlayers[nIdx]->isHavePlayer() )
		{
			bool bW = (m_vTaxasPlayers[nIdx]->getPlayerData().nStateFlag & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate ;
			if ( bW )
			{
				m_vTaxasPlayers[nIdx]->showBestCard(getPokerData()->vPublicCardNums);
			}
		}
	}

	if ( m_pLocalPlayer->getRoot()->isVisible())
	{
		bool bW = (m_pLocalPlayer->getPlayerData().nStateFlag & eRoomPeer_WaitCaculate) == eRoomPeer_WaitCaculate ;
		if ( bW )
		{
			m_pLocalPlayer->showBestCard(getPokerData()->vPublicCardNums);
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

	Node* pStartNode = m_vVicePool[nPoolIdx];
	if ( getPokerData()->vVicePool[nPoolIdx] == 0 )
	{
		pStartNode = m_pMainPool ;
	}

	Vec2 pt = pStartNode->getParent()->convertToWorldSpace(m_vVicePool[nPoolIdx]->getPosition());
	pStartNode->setVisible(false);
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
	if ( eState >= eRoomState_TP_MAX )
	{
		return ;
	}

	CCLOG("go to state = %d",eState);
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
			m_pLocalPlayer->setActCallBack(CC_CALLBACK_3(CTaxasPokerBettingState::onLocalPlayerActCallBack,(CTaxasPokerBettingState*)m_vAllState[m_eCurState]));
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
	CCLOG("enter new state = %d",m_eCurState);
}

void CTaxasPokerScene::onClickPlayerPhoto(CTaxasPlayer*pPlayer)
{
	if ( pPlayer->isHavePlayer() )
	{
		CCLOG("request player detail uid = %d",pPlayer->getPlayerData().nUserUID);
		stMsgTaxasPlayerStandUp msg ;
		msg.nRoomID = getPokerData()->nRoomID ;
		sendMsg(&msg,sizeof(msg));
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

void CTaxasPokerScene::refreshForBetRoundEnd()
{
	getPokerData()->resetBetRoundState() ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vTaxasPlayers[nIdx]->refreshForBetRoundEnd();
	}
	m_pLocalPlayer->refreshForBetRoundEnd();
}

void CTaxasPokerScene::refreshForGameEnd()
{
	getPokerData()->resetRuntimeData();

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vTaxasPlayers[nIdx]->refreshForGameEnd();
	}
	m_pLocalPlayer->refreshForGameEnd();

	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD ; ++nIdx )
	{
		m_vPublicCard[nIdx]->setVisible(false);
	}

	m_pMainPool->setVisible(false);

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		m_vVicePool[nIdx]->setVisible(false);
	}
}

void CTaxasPokerScene::refreshForGameStart()
{
	refreshForGameEnd();
}

void CTaxasPokerScene::refreshForWaitGame()
{
	refreshForGameEnd();
}

uint8_t CTaxasPokerScene::getPlayerCntWithState(eRoomPeerState eS )
{
	uint8_t nCnt = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( m_vTaxasPlayers[nIdx]->getRoot()->isVisible() && m_vTaxasPlayers[nIdx]->isHavePlayer() )
		{
			if ( m_vTaxasPlayers[nIdx]->isHaveState(eRoomPeer_WaitCaculate) )
			{
				++nCnt ;
			}
		}
	}

	if ( m_pLocalPlayer->getRoot()->isVisible())
	{
		if ( m_pLocalPlayer->isHaveState(eRoomPeer_WaitCaculate) )
		{
			++nCnt ;
		}
	}
	return nCnt ;
}

void CTaxasPokerScene::onEnterRoom(GotyeStatusCode code, GotyeRoom& room)
{
	CCLOG("enter chat room ret = %d",code);
}

void CTaxasPokerScene::chatInputCallBack(int nContentType , const char* pContent )
{
	//GotyeRoom room (14361018);
	//GotyeMessage msg = GotyeMessage::createTextMessage(room,pContent) ;
	//GotyeAPI::getInstance()->sendMessage(msg);
		
	//bool b ;
	//onReceiveMessage(msg,&b);
}
                      
void CTaxasPokerScene::onReceiveMessage(const GotyeMessage& message, bool* downloadMediaIfNeed)
{
	//if ( message.hasMedia() )
	//{
	//	*downloadMediaIfNeed = true ;
	//	return ;
	//}

	stChatItem* pItem = new stChatItem ;
	pItem->strName = message.sender.name ;
	pItem->strContent = message.text ;
	m_pChatLayer->pushChatItem(pItem);
}

void CTaxasPokerScene::onDownloadMediaInMessage(GotyeStatusCode code, const GotyeMessage& message)
{
	/*GotyeAPI::getInstance()->playMessage(message);*/
	stChatItem* pItem = new stChatItem ;
	pItem->strName = message.sender.name ;
	pItem->strContent = "Audio" ;
	m_pChatLayer->pushChatItem(pItem);
}

void CTaxasPokerScene::onReconnecting(GotyeStatusCode code, const GotyeLoginUser& user)
{
	if ( code == GotyeStatusCodeOK )
	{
		enterChatRoom() ; 
	}
	CCLOG("reconnect chat svr ret = %d ",code);
}

void CTaxasPokerScene::onExit()
{
	IBaseScene::onExit() ;
	//GotyeRoom room (getPokerData()->nChatRoomID);
	//GotyeAPI::getInstance()->leaveRoom(room);
}