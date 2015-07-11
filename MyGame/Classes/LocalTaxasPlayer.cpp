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
	lpfuncLocalAct = nullptr;
	m_bShowActBtn = false ;

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
		}
	}
	
	return true ;
}

void CLocalTaxasPlayer::onWaitAction()
{
	CTaxasPlayer::onWaitAction();
	// show btn ;
	m_pAction->play("showBtn",false);
	m_bShowActBtn = true ;
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
		lpfuncLocalAct(this,act,200);
	}

	m_pAction->play("hideBtn",false);
	m_bShowActBtn = false ;
}

void CLocalTaxasPlayer::onPrivateCard(uint8_t nIdx )
{
	// show real card
	if ( nIdx < TAXAS_PEER_CARD )
	{
		uint8_t nCompsiteNum = getPlayerData().vHoldCard[nIdx];
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

void CLocalTaxasPlayer::hideActBtns()
{
	if ( m_bShowActBtn )
	{
		m_bShowActBtn = false ;
		m_pAction->play("hideBtn",false);
	}
}
