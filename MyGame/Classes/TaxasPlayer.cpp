#include "TaxasPlayer.h"
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
	m_pName = (Label*)pRoot->getChildByName("name");
	m_pCoin = (Label*)pRoot->getChildByName("coin");
	m_pHoldCard[0] = (Sprite*)pRoot->getChildByName("holdCard0");
	m_pHoldCard[1] = (Sprite*)m_pHoldCard[0]->getChildByName("holdCard1");
	m_pTime = (Label*)pRoot->getChildByName("time");
	m_pState = (Label*)(pRoot->getChildByName("stateBg")->getChildByName("state"));
	m_pBetCoin = (Label*)(pRoot->getChildByName("betCoinBg")->getChildByName("betCoin"));
	m_nLocalIdx = nPosIdx;
	m_nSvrIdx = nPosIdx;

	Sprite* pbg = (Sprite*)pRoot->getChildByName("f_1");
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [pbg](Touch*ptouch, Event*)->bool
	{
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

	m_pTime->getParent()->setVisible(false);
	m_pName->setVisible(bHaveGuy);
	m_pCoin->setVisible(bHaveGuy);
	m_pHoldCard[0]->setVisible(bHaveGuy && bInGame);
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

	m_pBetCoin->getParent()->setVisible(m_pBindPlayerData->nBetCoinThisRound > 0);
	if ( m_pBindPlayerData->nBetCoinThisRound > 0 )
	{
		m_pBetCoin->setString(String::createWithFormat("%I64d",m_pBindPlayerData->nBetCoinThisRound)->getCString()) ;
	}
	m_pCoin->setString(String::createWithFormat("%I64d",m_pBindPlayerData->nTakeInMoney)->getCString()) ;
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

void CTaxasPlayer::onPrivateCard(uint8_t nIdx , uint16_t nCompsiteNum )
{
	if (nIdx < TAXAS_PEER_CARD  )
	{
		m_pHoldCard[nIdx]->setVisible(nCompsiteNum != 0 );
		m_pBindPlayerData->vHoldCard[nIdx] = nCompsiteNum ;
	}
	else
	{
		cocos2d::MessageBox("too big card idx ","error");
	}
}

void CTaxasPlayer::setClickPhotoCallBack(std::function<void(CTaxasPlayer*)>& lpFunc )
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
