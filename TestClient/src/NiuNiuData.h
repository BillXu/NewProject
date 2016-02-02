#pragma once
#include "NiuNiuMessageDefine.h"
struct stNiuNiuData
{
public:
	uint32_t nRoomID ;
	uint8_t nBankerIdx ;
	uint32_t nBottomBet ;
	uint8_t nBankerBetTimes ;
	uint8_t nRoomState ;
	uint32_t nDeskFee ;
	stNNRoomInfoPayerItem vPlayers[MAX_PEERS_IN_TAXAS_ROOM];
public:
	stNiuNiuData();
	uint32_t getBaseBet(){ return nBottomBet ;}
	uint32_t getFinalBaseBet();
	void onStartNewGame();
	uint8_t getSitDownPlayerCnt();
	uint32_t getLeftCanBetCoin();
	bool onMsg(stMsg* pmsg);
	stNNRoomInfoPayerItem* getPlayerByIdx(uint8_t nIdx );
	bool isHaveNiu(uint8_t nIdx );
	uint8_t getRandEmptySeatIdx();
};