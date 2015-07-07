#pragma once
#include "IBaseScene.h"
#include "TaxasPokerData.h"
class CTaxasPlayer;
class CLocalTaxasPlayer;
class CTaxasPokerScene
	:public IBaseScene
{
public:
	static Scene* createScene();
	CREATE_FUNC(CTaxasPokerScene);
	bool init();
	bool onMsg(stMsg* pmsg );
	void refreshContent();
	void refreshVicePools();
	void relayoutPlayerLocalPos( uint8_t nLocalPlayerSvrIdx );
	int8_t getLocalIdxBySvrIdx( uint8_t nSvrIdx );
	CTaxasPlayer* getTaxasPlayerBySvrIdx(uint8_t nSvrIdx );
	stTaxasPokerData* getPokerData(){ return &m_tGameData ;};
	void distributePrivateCard();
	CLocalTaxasPlayer* getLocalPlayer(){ return m_pLocalPlayer ;}
	void playersBetCoinGoMainPool();
	void onPlayersBetCoinArrived( uint8_t nNewVicePoolCnt );
	void onPlayerGiveupCoinArrived();
	Vec2 getMainPoolWorldPos();
	void distributePublicCard(uint8_t nRound ); // invalid value , 1 , 2 ,3 
	void showAllPlayersFinalCard();
	void winCoinGoToWinners(uint8_t nPoolIdx,uint64_t nCoinPerWinner,uint8_t vWinnerIdx[MAX_PEERS_IN_TAXAS_ROOM],uint8_t nWinnerCnt);
	void goToState(eRoomState eState,stMsg* pmsg = nullptr );
protected:
	void doLayoutTaxasPlayer(CTaxasPlayer*pPlayer,uint8_t nOffsetIdx );
protected:
	Sprite* m_vPublicCard[TAXAS_PUBLIC_CARD];
	Label* m_pMainPool;
	Label* m_vVicePool[MAX_PEERS_IN_TAXAS_ROOM];
	
	CTaxasPlayer* m_vTaxasPlayers[MAX_PEERS_IN_TAXAS_ROOM];
	Vec2 m_vPosOfTaxasPlayers[MAX_PEERS_IN_TAXAS_ROOM];
	CLocalTaxasPlayer* m_pLocalPlayer ;
	stTaxasPokerData m_tGameData;
	Vec2 m_ptMailPoolWorldPt ;
};