#pragma once
#include "cocos2d.h"
#include "CommonDefine.h"
#include "CommonData.h"
USING_NS_CC ;
class CTaxasPlayer
	:public Node  // just for autorelease with render tree , no other meannings ;
{
public:
	static CTaxasPlayer* create(Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tBindedPlayerData );
	virtual bool init(Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData);
	void doLayoutforPos();
	void refreshContent();
	void onAct(uint16_t nAct , uint32_t nValue );
	//void bindPlayerData(stTaxasPeerBaseData* tPlayerData );
	virtual void onWaitAction();
	bool isHavePlayer(){ return m_pBindPlayerData->nUserUID != 0 ;}
	stTaxasPeerBaseData& getPlayerData(){ return *m_pBindPlayerData ; }
	void setLocalIdx(int8_t nPosIdx );
	int8_t getLocalIdx();
	void setServerIdx(int8_t nSvrIdx );
	int8_t getServerIdx();
	virtual void onPrivateCard(uint8_t nIdx , uint16_t nCompsiteNum );
	void setClickPhotoCallBack(std::function<void(CTaxasPlayer*)>& lpFunc );
	Node* getRoot();
	void setPos(Vec2& pt );
	void betBlind(uint32_t nValue ){}
	void betCoinGoToMainPool(Vec2& ptMainPoolWorldPt ){}
	void showFinalCard();
	void onWinCoinArrived( uint64_t nWinCoin );
protected:
	Node* m_pRoot ;
	stTaxasPeerBaseData* m_pBindPlayerData; // CTaxasPlayer will display and serve for m_pBindPlayerData all life , but will not change m_pBindPlayerData 
	Label *m_pName ,*m_pState , *m_pTime, *m_pCoin,*m_pBetCoin;
	Sprite* m_pHoldCard[TAXAS_PEER_CARD];
	int8_t m_nTimeCountDown;
	int8_t m_nLocalIdx ;  // this idx will change , depoend on local player sit ;becasuse local player idx always 8 ;
	int8_t m_nSvrIdx ;  // this idx will not change ;
	std::function<void(CTaxasPlayer*)> lpFuncClick;
};