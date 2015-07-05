#include "LocalTaxasPlayer.h"
#include "CardPoker.h"
#include "ClientApp.h"
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
	m_pAction = CSLoader::getInstance()->createTimeline("res/PlayerSelf.csb");
	pRoot->runAction(m_pAction);

	m_pbtnAllIn = (ui::Button*)pRoot->getChildByName("allIn");
	m_pbtnCheck = (ui::Button*)pRoot->getChildByName("check");
	m_pbtnAdd = (ui::Button*)pRoot->getChildByName("add");
	m_pbtnFollow = (ui::Button*)pRoot->getChildByName("follow");
	m_pbtnGiveUp = (ui::Button*)pRoot->getChildByName("giveUp");

	m_pbtnAllIn->addClickEventListener(CC_CALLBACK_1(CLocalTaxasPlayer::onClickBtn,this));
	m_pbtnCheck->addClickEventListener(CC_CALLBACK_1(CLocalTaxasPlayer::onClickBtn,this));
	m_pbtnFollow->addClickEventListener(CC_CALLBACK_1(CLocalTaxasPlayer::onClickBtn,this));
	m_pbtnGiveUp->addClickEventListener(CC_CALLBACK_1(CLocalTaxasPlayer::onClickBtn,this));
	return true ;
}

void CLocalTaxasPlayer::onWaitAction()
{
	CTaxasPlayer::onWaitAction();
	// show btn ;
	m_pAction->play("showBtn",false);
}

void CLocalTaxasPlayer::onClickBtn(cocos2d::Ref* pBtn)
{
	// send msg and hide btn ;
	m_pAction->play("showBtn",true);
}

void CLocalTaxasPlayer::onPrivateCard(uint8_t nIdx , uint16_t nCompsiteNum )
{
	// show real card
	if ( nIdx < TAXAS_PEER_CARD )
	{
		m_pHoldCard[nIdx]->setVisible(true);
		if (nCompsiteNum >= 53 )
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
