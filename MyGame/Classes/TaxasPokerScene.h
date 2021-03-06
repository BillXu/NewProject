#pragma once
#include "IBaseScene.h"
#include "TaxasPokerData.h"
class CTaxasPlayer;
class CLocalTaxasPlayer;
class CTaxasPokerSceneStateBase ;
class  CChatLayer ;
 
class CTaxasPokerScene
	:public IBaseScene
{
public:
	static Scene* createScene();
	CREATE_FUNC(CTaxasPokerScene);
	bool init();
	bool onMsg(stMsg* pmsg ); 
	void onRecievedRoomInfo();
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
	void distributePublicCard(uint8_t nRound ); // invalid value , 0 , 1 ,2
	void showAllPlayersFinalCard();
	void winCoinGoToWinners(uint8_t nPoolIdx,uint64_t nCoinPerWinner,uint8_t vWinnerIdx[MAX_PEERS_IN_TAXAS_ROOM],uint8_t nWinnerCnt);
	void onClickPlayerPhoto(CTaxasPlayer*pPlayer);
	void refreshForBetRoundEnd();
	void refreshForGameEnd();
	void refreshForGameStart();
	void refreshForWaitGame();
	uint8_t getCurRoomState(){return m_eCurState ;}
	uint8_t getPlayerCntWithState(eRoomPeerState eS );

	void chatInputCallBack(int nContentType , const char* pContent );
	virtual void onEnterRoom(GotyeStatusCode code, GotyeRoom& room);
	void onReceiveMessage(const GotyeMessage& message, bool* downloadMediaIfNeed);
	void onDownloadMediaInMessage(GotyeStatusCode code, const GotyeMessage& message);
	void onReconnecting(GotyeStatusCode code, const GotyeLoginUser& user);
	void onExit();
protected:
	void doLayoutTaxasPlayer(CTaxasPlayer*pPlayer,uint8_t nOffsetIdx );
	void goToState(eRoomState eState,stMsg* pmsg = nullptr );
	void enterChatRoom();
protected:
	CChatLayer* m_pChatLayer ;
	Sprite* m_vPublicCard[TAXAS_PUBLIC_CARD];
	ui::Text* m_pMainPool;
	ui::Text* m_vVicePool[MAX_PEERS_IN_TAXAS_ROOM];
	
	CTaxasPlayer* m_vTaxasPlayers[MAX_PEERS_IN_TAXAS_ROOM];
	Vec2 m_vPosOfTaxasPlayers[MAX_PEERS_IN_TAXAS_ROOM];
	CLocalTaxasPlayer* m_pLocalPlayer ;
	stTaxasPokerData m_tGameData;
	Vec2 m_ptMailPoolWorldPt ;

	CTaxasPokerSceneStateBase* m_vAllState[eRoomState_TP_MAX];
	eRoomState m_eCurState;
};