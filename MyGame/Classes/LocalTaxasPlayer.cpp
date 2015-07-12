#include "LocalTaxasPlayer.h"
#include "CardPoker.h"
#include "ClientApp.h"
#define TIME_SWITCH_ACT_BTN 0.25
CLocalTaxasPlayer* CLocalTaxasPlayer::create(cocos2d::Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData )
{
	CLocalTaxasPlayer* p = new CLocalTaxasPlayer ;
	p->init(pRoot,nPosIdx,tPlayerData);
	p->autorelease() ;
	return p ;
}

bool CLocalTaxasPlayer::init(cocos2d::Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData)
{
	CTaxasPlayer::init(pRoot,nPosIdx,tPlayerData);
	//m_pAction = CSLoader::getInstance()->createTimeline("res/PlayerSelf.csb");
	//pRoot->runAction(m_pAction);
	lpfuncLocalAct = nullptr;

	memset(m_vbtnAct,0,sizeof(m_vbtnAct)) ;
	m_vbtnAct[eRoomPeerAction_AllIn] = (ui::Button*)pRoot->getChildByName("allIn");
	m_vbtnAct[eRoomPeerAction_Pass] = (ui::Button*)pRoot->getChildByName("check");
	m_vbtnAct[eRoomPeerAction_Add] = (ui::Button*)pRoot->getChildByName("add");
	m_vbtnAct[eRoomPeerAction_Follow] = (ui::Button*)pRoot->getChildByName("follow");
	m_vbtnAct[eRoomPeerAction_GiveUp] = (ui::Button*)pRoot->getChildByName("giveUp");

	for ( uint8_t nIdx = 0 ; nIdx < eRoomPeerAction_Max ; ++nIdx )
	{
		if ( m_vbtnAct[nIdx] )
		{
			m_vbtnAct[nIdx]->addClickEventListener(CC_CALLBACK_1(CLocalTaxasPlayer::onClickBtn,this));
			m_vbtnAct[nIdx]->setVisible(false);
			m_vptBtnAct[nIdx] = m_vbtnAct[nIdx]->getPosition();
			m_vbtnAct[nIdx]->setPosition(Vec2::ZERO);
		}
	}
	
	return true ;
}

void CLocalTaxasPlayer::onWaitAction(uint64_t nCurMostBetCoin)
{
	CTaxasPlayer::onWaitAction(nCurMostBetCoin);
	// show btn ;
	showActBtns(nCurMostBetCoin);
}

void CLocalTaxasPlayer::onAct(uint16_t nAct , uint32_t nValue )
{
	CTaxasPlayer::onAct(nAct,nValue);
	hideActBtns();
}

void CLocalTaxasPlayer::onClickBtn(cocos2d::Ref* pBtn)
{
	Node* ppd = (Node*)pBtn;
	// send msg and hide btn ;
	uint8_t act = 0 ;
	for ( ; act < eRoomPeerAction_Max ; ++act )
	{
		if ( m_vbtnAct[act] == pBtn )
		{
			break; 
		}
	}
	if ( lpfuncLocalAct )
	{
		lpfuncLocalAct(this,act,0);
	}
	hideActBtns();
}

void CLocalTaxasPlayer::onPrivateCard(uint8_t nIdx )
{
	if ( isHavePlayer() == false )
	{
		return ;
	}
	// show real card
	if ( nIdx < TAXAS_PEER_CARD )
	{
		uint8_t nCompsiteNum = getPlayerData().vHoldCard[nIdx];
		m_pHoldCard[nIdx]->setVisible(true);
		if (nCompsiteNum >= 53 || nCompsiteNum == 0  )
		{
			CCLOG("my private card error = %d\n",nCompsiteNum );
			nCompsiteNum = 2;
		}
		m_pHoldCard[nIdx]->setSpriteFrame(CClientApp::getCardSpriteByCompsiteNum(nCompsiteNum));
	}
	else
	{
		CCLOG("my private card idx error = %d\n",nIdx );
	}
}

void CLocalTaxasPlayer::bindPlayerData(stTaxasPeerBaseData* tPlayerData )
{
	m_pBindPlayerData = tPlayerData;
}

void CLocalTaxasPlayer::hideActBtns()
{
	if ( m_vbtnAct[eRoomPeerAction_Add]->isVisible() == false )
	{
		return ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < eRoomPeerAction_Max; ++nIdx )
	{
		Node* p = m_vbtnAct[nIdx] ;
		if (!p)
		{
			continue;
		}
		
		MoveTo* pto = MoveTo::create(TIME_SWITCH_ACT_BTN,Vec2::ZERO);
		ScaleTo* ptos = ScaleTo::create(TIME_SWITCH_ACT_BTN,0);
		Spawn* ps = Spawn::createWithTwoActions(pto,ptos);
		Hide* phide = Hide::create();
		Sequence* pseq = Sequence::createWithTwoActions(ps,phide);
		p->runAction(pseq);
	}
}

void CLocalTaxasPlayer::showActBtns(uint64_t nCurMostBetCoin )
{
	if ( m_vbtnAct[eRoomPeerAction_Add]->isVisible() )
	{
		return ;
	}

	for ( uint8_t nIdx = 0 ; nIdx < eRoomPeerAction_Max; ++nIdx )
	{
		Node* p = m_vbtnAct[nIdx] ;
		if (!p)
		{
			continue;
		}
		p->setVisible(nIdx != eRoomPeerAction_Follow && nIdx != eRoomPeerAction_Pass );
		p->setScale(0);
		MoveTo* pto = MoveTo::create(TIME_SWITCH_ACT_BTN,m_vptBtnAct[nIdx]);
		ScaleTo* ptos = ScaleTo::create(TIME_SWITCH_ACT_BTN,1);
		Spawn* ps = Spawn::createWithTwoActions(pto,ptos);
		p->runAction(ps);
	}

	m_vbtnAct[eRoomPeerAction_Follow]->setVisible(nCurMostBetCoin > getPlayerData().nBetCoinThisRound) ;
	m_vbtnAct[eRoomPeerAction_Pass]->setVisible(!m_vbtnAct[eRoomPeerAction_Follow]->isVisible());
}
