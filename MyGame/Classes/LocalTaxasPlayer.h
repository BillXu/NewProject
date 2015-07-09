#pragma once
#include "TaxasPlayer.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
class CLocalTaxasPlayer
	:public CTaxasPlayer
{
public:
	static CLocalTaxasPlayer* create(cocos2d::Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData );
	bool init(cocos2d::Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData);
	virtual void onWaitAction();
	void onClickBtn(cocos2d::Ref* pBtn);
	void onPrivateCard(uint8_t nIdx);
	void bindPlayerData(stTaxasPeerBaseData* tPlayerData );
protected:
	ui::Button *m_pbtnFollow,*m_pbtnCheck,*m_pbtnAdd,*m_pbtnGiveUp,*m_pbtnAllIn;
	cocostudio::timeline::ActionTimeline *m_pAction;
};