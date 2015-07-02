#include "TaxasPokerScene.h"
#include "LocalTaxasPlayer.h"
#include "ClientApp.h"
#include "TaxasMessageDefine.h"
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
		if ( m_vTaxasPlayers[nIdx]->isVisible() == false )
		{
			continue;
		}

		m_vTaxasPlayers[nIdx]->refreshContent();
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

}