#pragma once
#include "CommonDefine.h"
#include "CommonData.h"
struct stMsg ;
struct stTaxasPokerData
{
	uint32_t nRoomID ;
	uint8_t nMaxSeat;
	uint32_t nLittleBlind;
	uint32_t nMiniTakeIn ;
	uint64_t nMaxTakeIn ;
	// running members ;
	uint32_t eCurRoomState ; // eeRoomState ;
	uint8_t nBankerIdx ;
	uint8_t nLittleBlindIdx ;
	uint8_t nBigBlindIdx ;
	int8_t nCurWaitPlayerActionIdx ;
	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;
	uint8_t vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
	uint64_t vVicePool[MAX_PEERS_IN_TAXAS_ROOM] ;
	stTaxasPeerBaseData vAllTaxasPlayerData[MAX_PEERS_IN_TAXAS_ROOM] ;
public:
	uint8_t getVicePoolCnt();
	void resetRuntimeData();
	stTaxasPeerBaseData* getTaxasPlayerData( uint8_t nSvrIdx);
	void setTaxasPlayerData(stTaxasPeerBaseData* pdata );
	bool onMsg(stMsg* pmsg );
	uint32_t getPlayerAddCoinLowLimit( uint8_t nPlayerSvrIdx );
	uint64_t getPlayerAddCoinUpLimit( uint8_t nPlayerSvrIdx);
};
