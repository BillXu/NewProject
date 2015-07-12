#pragma once
#include "cocos2d.h"
#include "CommonDefine.h"
#include "CommonData.h"
#include "ui/CocosGUI.h"
USING_NS_CC ;
class CChipGroup;
class CTaxasPokerScene;
class CTaxasPlayer
	:public Node  // just for autorelease with render tree , no other meannings ;
{
public:
	static CTaxasPlayer* create(Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tBindedPlayerData );
	virtual bool init(Node* pRoot,int8_t nPosIdx,stTaxasPeerBaseData* tPlayerData);
	void createCoinAni(Node* pSceneLayer ); // coin ani must add to scene layer ,otherwise will be hide by public cards
	void doLayoutforPos();
	void refreshContent( CTaxasPokerScene* pScene );
	virtual void onAct(uint16_t nAct , uint32_t nValue );
	//void bindPlayerData(stTaxasPeerBaseData* tPlayerData );
	virtual void onWaitAction(uint64_t nCurMostBetCoin );
	bool isHavePlayer(){ return m_pBindPlayerData->nUserUID != 0 ;}
	stTaxasPeerBaseData& getPlayerData(){ return *m_pBindPlayerData ; }
	void setLocalIdx(int8_t nPosIdx );
	int8_t getLocalIdx();
	void setServerIdx(int8_t nSvrIdx );
	int8_t getServerIdx();
	virtual void onPrivateCard(uint8_t nIdx );
	void setClickPhotoCallBack(std::function<void(CTaxasPlayer*)> lpFunc );
	Node* getRoot();
	void setPos(Vec2& pt );
	void betBlind(uint32_t nValue );
	bool betCoinGoToMainPool(Vec2& ptMainPoolWorldPt, float fAni );
	void winCoinGoToPlayer( Vec2& ptWinPoolWorldPt, float fAni );
	bool isHaveState(eRoomPeerState eS);
	void distributeHoldCard(Vec2& ptWorldPt,uint8_t nIdx , float fAniTime, float fDelay );
	void showBestCard(uint8_t vPublicCard[MAX_TAXAS_HOLD_CARD],float fAniTime = TIME_TAXAS_SHOW_BEST_CARD );
	void refreshForBetRoundEnd();
	void refreshForGameEnd();
	void refreshForGameStart();
	void refreshForWaitGame();
protected:
	void doBetCoinAni();
	void refreshCoin(CChipGroup* pGrop = nullptr );
	void setBetCoin(uint64_t nBetCoin );
	void setActState(uint8_t nAct );
	virtual void stopTimeClock();
protected:
	Node* m_pRoot ;
	stTaxasPeerBaseData* m_pBindPlayerData; // CTaxasPlayer will display and serve for m_pBindPlayerData all life , but will not change m_pBindPlayerData 
	ui::Text *m_pName ,*m_pState , *m_pTime, *m_pCoin,*m_pBetCoin, *m_pCardType;
	Sprite* m_pHoldCard[TAXAS_PEER_CARD];
	Vec2 m_vHoldCardPt[TAXAS_PEER_CARD];
	float m_vHoldCardRot[TAXAS_PEER_CARD];
	Sprite* m_vBesetCard[MAX_TAXAS_HOLD_CARD];
	Vec2 m_vPtBestCard[MAX_TAXAS_HOLD_CARD];
	int8_t m_nTimeCountDown;
	int8_t m_nLocalIdx ;  // this idx will change , depoend on local player sit ;becasuse local player idx always 8 ;
	int8_t m_nSvrIdx ;  // this idx will not change ;
	std::function<void(CTaxasPlayer*)> lpFuncClick;

	CChipGroup* m_pBetCoinAni, *m_pCoinGoToMainlPoolAni;
	CChipGroup* m_vWinCoinAni[MAX_PEERS_IN_TAXAS_ROOM];
};