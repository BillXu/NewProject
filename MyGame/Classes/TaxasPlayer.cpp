#include "TaxasPlayer.h"
#include "ChipGroup.h"
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
	m_nLocalIdx = nPosIdx;
	m_nSvrIdx = nPosIdx;

	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PEER_CARD ; ++nIdx )
	{
		m_vHoldCardRot[nIdx] = m_pHoldCard[nIdx]->getRotation();
		m_vHoldCardPt[nIdx] = m_pHoldCard[nIdx]->getPosition();
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

void CTaxasPlayer::refreshContent()
{
	bool bHaveGuy = isHavePlayer();
	stTaxasPeerBaseData& tPlayerData = *m_pBindPlayerData;
	bool bInGame = (tPlayerData.nStateFlag & eRoomPeer_StayThisRound) == eRoomPeer_StayThisRound;

	m_pTime->setVisible(false);
	m_pName->setVisible(bHaveGuy);
	m_pCoin->setVisible(bHaveGuy);
	m_pHoldCard[0]->setVisible(bHaveGuy && bInGame);
	m_pHoldCard[1]->setVisible(bHaveGuy && bInGame);
	m_pBetCoin->getParent()->setVisible(bHaveGuy && bInGame);
	m_pState->getParent()->setVisible(bHaveGuy && bInGame);
	
	if ( bHaveGuy == false )
	{
		return ;
	}

	m_pCoin->setString(String::createWithFormat("%I64d",tPlayerData.nTakeInMoney)->getCString()) ;
	m_pName->setString(tPlayerData.cName);

	if ( bInGame )
	{
		m_pState->getParent()->setVisible(tPlayerData.eCurAct != eRoomPeerAction_None );
		if ( tPlayerData.eCurAct != eRoomPeerAction_None )
		{
			onAct(tPlayerData.eCurAct,0) ;
		}
	}
}

void CTaxasPlayer::onAct(uint16_t nAct , uint32_t nValue )
{
	m_pTime->setVisible(false);
	Director::getInstance()->getScheduler()->unschedule(SCHEDULE_FUNC_NAME_FOR_TIMER,this);

	m_pState->getParent()->setVisible(eRoomPeerAction_None != nAct );
	bool bDoBet = false ;
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
			bDoBet = true ;
		}
		break;
	case eRoomPeerAction_Follow:
		{
			m_pState->setString("Follow");
			bDoBet = true ;
		}
		break;
	case eRoomPeerAction_Add:
		{
			m_pState->setString("Add");
			bDoBet = true ;
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

	if ( bDoBet )
	{
		doBetCoinAni();
	}
}

//void CTaxasPlayer::setPlayerData(stTaxasPeerBaseData* tPlayerData )
//{
//	m_pBindPlayerData = tPlayerData ;
//	refreshContent();
//}

void CTaxasPlayer::onWaitAction()
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
	if ( m_pBetCoin->getParent()->isVisible() )
	{
		CCLOG("do not bet , so no go pool ani");
		m_pState->setVisible(false); 
		refreshCoin();
		return false;
	}

	if ( nullptr == m_pCoinGoToMainlPoolAni )
	{
		m_pCoinGoToMainlPoolAni = CChipGroup::CreateGroup();
		m_pCoinGoToMainlPoolAni->SetFinishCallBack([=](CChipGroup*){ m_pState->setVisible(false); refreshCoin(); });
		m_pCoinGoToMainlPoolAni->setPosition(Vec2::ZERO) ;
		m_pBetCoin->addChild(m_pCoinGoToMainlPoolAni);
	}
	m_pCoinGoToMainlPoolAni->SetGroupCoin(8,false );
	Vec2 pt = m_pCoinGoToMainlPoolAni->getParent()->convertToNodeSpace(ptMainPoolWorldPt);
	m_pCoinGoToMainlPoolAni->SetDestPosition(pt);
	m_pCoinGoToMainlPoolAni->Start(CChipGroup::eChipMove_Group2None,fAni);
	return true ;
}

void CTaxasPlayer::showFinalCard()
{

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
	m_pBetCoinAni->SetGroupCoin(3,false );
	Vec2 pt = m_pBetCoin->getPosition();
	pt = m_pBetCoin->getParent()->convertToWorldSpace(pt);
	pt = m_pBetCoinAni->getParent()->convertToNodeSpace(pt);
	m_pBetCoinAni->SetDestPosition(pt);
	m_pBetCoinAni->Start(CChipGroup::eChipMove_Group2None,0.1);
}

void CTaxasPlayer::refreshCoin(CChipGroup* pGrop)
{
	m_pBetCoin->getParent()->setVisible(m_pBindPlayerData->nBetCoinThisRound > 0);
	if ( m_pBindPlayerData->nBetCoinThisRound > 0 )
	{
		m_pBetCoin->setString(String::createWithFormat("%I64d",m_pBindPlayerData->nBetCoinThisRound)->getCString()) ;
	}
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

		if ( m_vWinCoinAni[nIdx] == nullptr )
		{
			pTarget = CChipGroup::CreateGroup();
			pTarget->SetFinishCallBack(CC_CALLBACK_1(CTaxasPlayer::refreshCoin,this));
			pTarget->setPosition(Vec2::ZERO) ;
			m_pCoin->addChild(pTarget);
			m_vWinCoinAni[nIdx] = pTarget ;
			pTarget->SetDestPosition(Vec2::ZERO);
			break;
		}
	}

	if ( pTarget == nullptr )
	{
		CCLOG("why so many win pool , do not have win ani");
		refreshCoin();
		return ;
	}
	pTarget->SetGroupCoin(3,false );
	Vec2 pt = pTarget->getParent()->convertToNodeSpace(ptWinPoolWorldPt);
	pTarget->SetOriginPosition(pt);
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

