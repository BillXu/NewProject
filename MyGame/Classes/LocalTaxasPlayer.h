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
	virtual void onWaitAction(uint64_t nCurMostBetCoin);
	virtual void onAct(uint16_t nAct , uint32_t nValue );
	void onClickBtn(cocos2d::Ref* pBtn);
	void onPrivateCard(uint8_t nIdx);
	void bindPlayerData(stTaxasPeerBaseData* tPlayerData );
	void setActCallBack( std::function<void(CTaxasPlayer* pPlayer ,uint8_t nActValue, uint32_t nValue)> func){ lpfuncLocalAct = func; }
	void hideActBtns();
	void showActBtns(uint64_t nCurMostBetCoin);
protected:
	ui::Button *m_vbtnAct[eRoomPeerAction_Max];
	Vec2 m_vptBtnAct[eRoomPeerAction_Max];
	std::function<void(CTaxasPlayer* pPlayer ,uint8_t nActValue, uint32_t nValue)> lpfuncLocalAct ;
};