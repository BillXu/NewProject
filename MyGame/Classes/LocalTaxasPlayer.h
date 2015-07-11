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
	virtual void onAct(uint16_t nAct , uint32_t nValue );
	void onClickBtn(cocos2d::Ref* pBtn);
	void onPrivateCard(uint8_t nIdx);
	void bindPlayerData(stTaxasPeerBaseData* tPlayerData );
	void setActCallBack( std::function<void(CTaxasPlayer* pPlayer ,uint8_t nActValue, uint32_t nValue)> func){ lpfuncLocalAct = func; }
	void hideActBtns();
protected:
	ui::Button *m_vbtnAct[eRoomPeerAction_Max];
	cocostudio::timeline::ActionTimeline *m_pAction;
	std::function<void(CTaxasPlayer* pPlayer ,uint8_t nActValue, uint32_t nValue)> lpfuncLocalAct ;
	bool m_bShowActBtn;
};