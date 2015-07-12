#include "TaxasPlayer.h"
#include "ChipGroup.h"
#include "TaxasPokerPeerCard.h"
#include "ClientApp.h"
#define SCHEDULE_FUNC_NAME_FOR_TIMER "timeCountDown"
CTaxasPlayer* CTaxasPlayer::create(Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData)
{
	CTaxasPlayer* p = new CTaxasPlayer ;
	p->init(pRoot,nPosIdx,tPlayerData) ;
	p->autorelease();
	return p ;
}

bool CTaxasPlayer::init(Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData)
{
	m_pBindPlayerData = tPlayerData ;
	lpFuncClick = nullptr ;
	m_pRoot = pRoot ;
	m_pCoinGoToMainlPoolAni = m_pBetCoinAni = nullptr ;
	memset(m_vWinCoinAni,0,sizeof(m_vWinCoinAni));
	m_pName = (ui::Text*)pRoot->getChildByName("name");
	m_pCoin = (ui::Text*)pRoot->getChildByName("coin");
	m_pHoldCard[0] = (Sprite*)pRoot->getChildByName("holdCard0");
	m_pHoldCard[1] = (Sprite*)pRoot->getChildByName("holdCard1");
	m_pTime = (ui::Text*)pRoot->getChildByName("time");
	m_pState = (ui::Text*)(pRoot->getChildByName("stateBg")->getChildByName("state"));
	m_pBetCoin = (ui::Text*)(pRoot->getChildByName("betCoinBg")->getChildByName("betCoin"));
	m_pCardType = (ui::Text*)pRoot->getChildByName("cardType");
	m_pCardType->setVisible(false);
	m_nLocalIdx = nPosIdx;
	m_nSvrIdx = nPosIdx;

	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PEER_CARD ; ++nIdx )
	{
		m_vHoldCardRot[nIdx] = m_pHoldCard[nIdx]->getRotation();
		m_vHoldCardPt[nIdx] = m_pHoldCard[nIdx]->getPosition();
	}

	// best card
	for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
	{
		m_vBesetCard[nIdx] = (Sprite*)pRoot->getChildByName(String::createWithFormat("bestCard%d",nIdx)->getCString()) ;
		m_vPtBestCard[nIdx] = m_vBesetCard[nIdx]->getPosition();
		m_vBesetCard[nIdx]->setVisible(false);  
	}

	Sprite* pbg = (Sprite*)pRoot->getChildByName("f_1");
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [pbg](Touch*ptouch, Event*)->bool
	{
		if ( pbg->isVisible() == false || pbg->getParent()->isVisible() == false )
			return false ;
		Vec2 pt = ptouch->getLocation();
		Rect box = pbg->getBoundingBox();
		pt = pbg->getParent()->convertToNodeSpace(pt);
		return box.containsPoint(pt);
	};
	listener->onTouchEnded = [this](Touch*ptouch, Event*)
	{
		if ( lpFuncClick )
		{
			lpFuncClick(this);
		}
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, pbg);
	doLayoutforPos();
	return true ;
}

void CTaxasPlayer::createCoinAni(Node* pSceneLayer )
{
	// create go main pool ani 
	if ( nullptr == m_pCoinGoToMainlPoolAni )
	{
		m_pCoinGoToMainlPoolAni = CChipGroup::CreateGroup();
		m_pCoinGoToMainlPoolAni->SetFinishCallBack([=](CChipGroup*){ setActState(eRoomPeerAction_None); } );
		m_pCoinGoToMainlPoolAni->setPosition(Vec2::ZERO) ;
		pSceneLayer->addChild(m_pCoinGoToMainlPoolAni);
	}

	// create win coin ani
	CChipGroup* pTarget = nullptr ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
	{
		if ( m_vWinCoinAni[nIdx] == nullptr )
		{
			pTarget = CChipGroup::CreateGroup();
			pTarget->SetFinishCallBack(CC_CALLBACK_1(CTaxasPlayer::refreshCoin,this));
			pTarget->setPosition(Vec2::ZERO) ;
			pSceneLayer->addChild(pTarget);
			m_vWinCoinAni[nIdx] = pTarget ;
			pTarget->SetDestPosition(Vec2::ZERO);
		}
	}
}

void CTaxasPlayer::doLayoutforPos()
{
	bool bFlip = m_nLocalIdx > 4 ;
	Sprite* pOper = (Sprite*)m_pState->getParent();
	pOper->setFlippedX(bFlip);

	float pt = m_pBetCoin->getParent()->getPosition().x;
	pt = abs(pt);
	if (bFlip)pt*=-1;
	m_pBetCoin->getParent()->setPositionX(pt);
}

void CTaxasPlayer::refreshContent( uint8_t nRoomCurState ,uint8_t vPublicCard[5] )
{
	refreshForGameEnd();
	bool bHaveGuy = isHavePlayer();
	stTaxasPeerBaseData& tPlayerData = *m_pBindPlayerData;
	bool bInGame = (tPlayerData.nStateFlag & eRoomPeer_StayThisRound) == eRoomPeer_StayThisRound;

	m_pTime->setVisible(false);
	m_pName->setVisible(bHaveGuy);
	m_pCoin->setVisible(bHaveGuy);
	m_pHoldCard[0]->setVisible(bHaveGuy && bInGame);
	m_pHoldCard[1]->setVisible(bHaveGuy && bInGame);
	setBetCoin(tPlayerData.nBetCoinThisRound);
	setActState(tPlayerData.eCurAct);
	
	if ( bHaveGuy == false )
	{
		return ;
	}

	m_pCoin->setString(String::createWithFormat("%I64d",tPlayerData.nTakeInMoney)->getCString()) ;
	m_pName->setString(tPlayerData.cName);

	bool bShowFinalCard = (eRoomState_TP_GameResult == nRoomCurState ) && isHaveState(eRoomPeer_WaitCaculate);
	if ( bShowFinalCard )
	{
		showBestCard(vPublicCard,0) ;
	}
	
}

void CTaxasPlayer::onAct(uint16_t nAct , uint32_t nValue )
{
	m_pTime->setVisible(false);
	Director::getInstance()->getScheduler()->unschedule(SCHEDULE_FUNC_NAME_FOR_TIMER,this);
	bool bDoBet = (nAct != eRoomPeerAction_Pass && eRoomPeerAction_GiveUp != nAct );
	setActState(nAct);
	if ( bDoBet )
	{
		doBetCoinAni();
	}
}

void CTaxasPlayer::onWaitAction(uint64_t nCurMostBetCoin)
{
	Director::getInstance()->getScheduler()->unschedule(SCHEDULE_FUNC_NAME_FOR_TIMER,this);
	m_pTime->setVisible(true) ;
	m_nTimeCountDown = TIME_TAXAS_BET ;
	Director::getInstance()->getScheduler()->schedule([=](float f )
	{
		--m_nTimeCountDown;
		if (m_nTimeCountDown < 0 ) m_nTimeCountDown = 0 ;
		m_pTime->setString(String::createWithFormat("%d",m_nTimeCountDown)->getCString()) ;

	},this,1.0,TIME_TAXAS_BET,0,false,SCHEDULE_FUNC_NAME_FOR_TIMER);
}

void CTaxasPlayer::setLocalIdx(int8_t nPosIdx )
{
	if ( m_nLocalIdx == nPosIdx )
		return ;

	m_nLocalIdx = nPosIdx ;
	m_nLocalIdx = m_nLocalIdx % MAX_PEERS_IN_TAXAS_ROOM ;
	doLayoutforPos();
}

int8_t CTaxasPlayer::getLocalIdx()
{
	return m_nLocalIdx ;
}

void CTaxasPlayer::setServerIdx(int8_t nSvrIdx )
{
	m_nSvrIdx = nSvrIdx ;
}

int8_t CTaxasPlayer::getServerIdx()
{
	return m_nSvrIdx ;
}

void CTaxasPlayer::onPrivateCard(uint8_t nIdx)
{
	if (nIdx < TAXAS_PEER_CARD  )
	{
		m_pHoldCard[nIdx]->setVisible( getPlayerData().vHoldCard[nIdx] != 0  );
	}
	else
	{
		cocos2d::MessageBox("too big card idx ","error");
	}
}

void CTaxasPlayer::setClickPhotoCallBack(std::function<void(CTaxasPlayer*)> lpFunc )
{
	lpFuncClick = lpFunc ;
}

Node* CTaxasPlayer::getRoot()
{
	return m_pRoot ;
}
void CTaxasPlayer::setPos(Vec2& pt )
{
	m_pRoot->setPosition(pt);
}

void CTaxasPlayer::betBlind(uint32_t nValue )
{
	doBetCoinAni();
}

bool CTaxasPlayer::betCoinGoToMainPool(Vec2& ptMainPoolWorldPt, float fAni )
{
	setBetCoin(0);
	CCLOG("coin go to main pool");
	if ( m_pCoinGoToMainlPoolAni == nullptr )
	{
		CCLOG("go main pool ani is null");
		return false ;
	}
	m_pCoinGoToMainlPoolAni->setVisible(true);
	m_pCoinGoToMainlPoolAni->SetGroupCoin(8,false );
	Vec2 pt = m_pCoinGoToMainlPoolAni->getParent()->convertToNodeSpace(ptMainPoolWorldPt);
	m_pCoinGoToMainlPoolAni->SetDestPosition(pt);
	pt = m_pBetCoin->getParent()->getPosition();
	pt = m_pBetCoin->getParent()->getParent()->convertToWorldSpace(pt);
	pt = m_pCoinGoToMainlPoolAni->getParent()->convertToNodeSpace(pt);
	m_pCoinGoToMainlPoolAni->SetOriginPosition(pt);
	m_pCoinGoToMainlPoolAni->Start(CChipGroup::eChipMove_Group2None,fAni);
	return true ;
}

void CTaxasPlayer::doBetCoinAni()
{
	if ( nullptr == m_pBetCoinAni )
	{
		m_pBetCoinAni = CChipGroup::CreateGroup();
		m_pBetCoinAni->SetFinishCallBack(CC_CALLBACK_1(CTaxasPlayer::refreshCoin,this));
		m_pBetCoinAni->setPosition(Vec2::ZERO) ;
		m_pCoin->addChild(m_pBetCoinAni);
	}
	m_pBetCoinAni->SetGroupCoin(2,false );
	Vec2 pt = m_pBetCoin->getParent()->getPosition();
	pt = m_pBetCoin->getParent()->getParent()->convertToWorldSpace(pt);
	pt = m_pBetCoinAni->getParent()->convertToNodeSpace(pt);
	m_pBetCoinAni->SetDestPosition(pt);
	m_pBetCoinAni->SetOriginPosition(Vec2::ZERO);
	m_pBetCoinAni->Start(CChipGroup::eChipMove_Group2None,TIME_PLAYER_BET_COIN_ANI);
}

void CTaxasPlayer::refreshCoin(CChipGroup* pGrop)
{
	CCLOG("refreshCoin bet coin this round = %I64d",m_pBindPlayerData->nBetCoinThisRound);
	setBetCoin(m_pBindPlayerData->nBetCoinThisRound);
	m_pCoin->setString(String::createWithFormat("%I64d",m_pBindPlayerData->nTakeInMoney)->getCString()) ;
}

void CTaxasPlayer::winCoinGoToPlayer( Vec2& ptWinPoolWorldPt, float fAni )
{
	CChipGroup* pTarget = nullptr ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM; ++nIdx )
	{
		if ( m_vWinCoinAni[nIdx] && m_vWinCoinAni[nIdx]->isRunningAni() == false )
		{
			pTarget = m_vWinCoinAni[nIdx];
			break;
		}
	}

	if ( pTarget == nullptr )
	{
		CCLOG("why so many win pool , do not have win ani");
		refreshCoin();
		return ;
	}

	pTarget->setVisible(true);
	pTarget->SetGroupCoin(8,false );
	Vec2 pt = pTarget->getParent()->convertToNodeSpace(ptWinPoolWorldPt);
	pTarget->SetOriginPosition(pt);

	pt = m_pCoin->getPosition();
	pt = m_pCoin->getParent()->convertToWorldSpace(pt);
	pt = pTarget->getParent()->convertToNodeSpace(pt);
	pTarget->SetDestPosition(pt);
	pTarget->Start(CChipGroup::eChipMove_Group2None,fAni);
}

bool CTaxasPlayer::isHaveState(eRoomPeerState eS)
{
	if ( isHavePlayer() == false )
	{
		return false ;
	}
	return ((getPlayerData().nStateFlag & eS) == eS );
}

void CTaxasPlayer::distributeHoldCard(Vec2& ptWorldPt, uint8_t nIdx , float fAniTime, float fDelay  )
{
	Sprite* ps = m_pHoldCard[nIdx] ;
	Vec2 pt = ps->getParent()->convertToNodeSpace(ptWorldPt);
	ps->setVisible(true);
	ps->setPosition(pt);
	ps->setRotation(0);
	DelayTime* pDelay = DelayTime::create(fDelay);
	MoveTo* ptTarget = MoveTo::create(fAniTime,m_vHoldCardPt[nIdx]);
	RotateTo* pRot = RotateTo::create(fAniTime,m_vHoldCardRot[nIdx]) ;
	Spawn* pa = Spawn::createWithTwoActions(ptTarget,pRot);
	CallFunc* pfunc = CallFunc::create([=](){onPrivateCard(nIdx);});
	Sequence* psq = Sequence::create(pDelay,pa,pfunc,nullptr);
	ps->stopAllActions();
	ps->runAction(psq);
}

void CTaxasPlayer::setBetCoin(uint64_t nBetCoin )
{
	m_pBetCoin->getParent()->setVisible( nBetCoin > 0);
	if ( nBetCoin > 0 )
	{
		m_pBetCoin->setString(String::createWithFormat("%I64d",nBetCoin)->getCString()) ;
	}
}

void CTaxasPlayer::setActState(uint8_t nAct )
{
	 m_pState->getParent()->setVisible(nAct != eRoomPeerAction_None);
	 switch ( nAct )
	 {
	 case eRoomPeerAction_Pass:
		 {
			 m_pState->setString("check");
		 }
		 break;
	 case eRoomPeerAction_AllIn:
		 {
			 m_pState->setString("All In");
		 }
		 break;
	 case eRoomPeerAction_Follow:
		 {
			 m_pState->setString("Follow");
		 }
		 break;
	 case eRoomPeerAction_Add:
		 {
			 m_pState->setString("Add");
		 }
		 break;
	 case eRoomPeerAction_GiveUp:
		 {
			 m_pState->setString("Give Up");
		 }
		 break;
	 default:
		 m_pState->setString("Unknown");
		 break;
	 } 
}

void CTaxasPlayer::showBestCard(uint8_t vPublicCard[MAX_TAXAS_HOLD_CARD], float fAniTime )
{
	if ( vPublicCard == nullptr )
	{
		CCLOG("public array card is null, can not show best card ");
		return ;
	}
	CTaxasPokerPeerCard peerCard ;
	if ( getPlayerData().vHoldCard[0] * getPlayerData().vHoldCard[1] <= 0 )
	{
		CCLOG("holde card is null , can not show best card");
		return ;
	}
	peerCard.AddCardByCompsiteNum(getPlayerData().vHoldCard[0]);
	peerCard.AddCardByCompsiteNum(getPlayerData().vHoldCard[1]);

	for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
	{
		if (vPublicCard[nIdx] == 0 )
		{
			CCLOG("public card have 0 value , so can not display best card");
			return ;
		}
		peerCard.AddCardByCompsiteNum(vPublicCard[nIdx]) ;
	}

	// display card
	m_pHoldCard[0]->setVisible(false);
	m_pHoldCard[1]->setVisible(false);
	m_pCardType->setVisible(false);
	unsigned char vFinalCard[MAX_TAXAS_HOLD_CARD] ;
	peerCard.GetFinalCard(vFinalCard) ;

	float fTimeCardMovePerPos = fAniTime / float(MAX_TAXAS_HOLD_CARD - 1 ) ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD ; ++nIdx )
	{
		m_vBesetCard[nIdx]->setVisible(true);
		m_vBesetCard[nIdx]->setSpriteFrame(CClientApp::getCardSpriteByCompsiteNum(vFinalCard[nIdx])) ;
		bool bHoldCard = ( vFinalCard[nIdx] == this->getPlayerData().vHoldCard[0] )|| (this->getPlayerData().vHoldCard[1] == vFinalCard[nIdx] );
		// if holde card give a speail card ;
		if (bHoldCard)
		{
			m_vBesetCard[nIdx]->setColor(Color3B::YELLOW);
		}
		else
		{
			m_vBesetCard[nIdx]->setColor(Color3B::WHITE);
		}

		// first card needn't move 
		m_vBesetCard[nIdx]->setPosition(m_vPtBestCard[0]) ;
		if ( nIdx != 0 )
		{
			MoveTo* pto = MoveTo::create(nIdx*fTimeCardMovePerPos,m_vPtBestCard[nIdx]);
			m_vBesetCard[nIdx]->runAction(pto);
		}
	}

	// after show all card display card type test ;
	m_pCardType->setString(peerCard.GetTypeName());
	DelayTime* pde = DelayTime::create(TIME_TAXAS_SHOW_BEST_CARD);
	CallFunc* pfunc = CallFunc::create([this](){ m_pCardType->setVisible(true);});
	Sequence* seq = Sequence::createWithTwoActions(pde,pfunc) ;
	runAction(seq);
}

void CTaxasPlayer::refreshForBetRoundEnd()
{
	setBetCoin(0);
	setActState(eRoomPeerAction_None);
}

void CTaxasPlayer::refreshForGameEnd()
{
	setBetCoin(0);
	setActState(eRoomPeerAction_None);
	m_pCardType->setVisible(false);
	for ( uint8_t nIdx = 0 ; nIdx < MAX_TAXAS_HOLD_CARD; ++nIdx )
	{
		m_vBesetCard[nIdx]->setVisible(false);
	}
	
	m_pHoldCard[0]->setVisible(false);
	m_pHoldCard[1]->setVisible(false);
}

void CTaxasPlayer::refreshForGameStart()
{
	refreshForGameEnd();
}

void CTaxasPlayer::refreshForWaitGame()
{
	refreshForGameEnd();
}
