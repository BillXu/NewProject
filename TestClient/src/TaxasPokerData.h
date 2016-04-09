#pragma once
#include "CommonDefine.h"
#include "CommonData.h"
#include "SitableRoomData.h"
#include <string>
struct stTaxasPlayer
	:public stSitableRoomPlayer
{
	uint32_t nCurAct ;
	uint32_t nBetCoinThisRound ;
	uint8_t vHoldCard[TAXAS_PEER_CARD];
	void betCoin(uint32_t nBetCoin )
	{
		nCoin -= nBetCoin ;
		nBetCoinThisRound += nBetCoin ;
	}

	void reset(){ stSitableRoomPlayer::reset(); nCurAct = eRoomPeerAction_None ; nBetCoinThisRound = 0 ; memset(vHoldCard,0,sizeof(vHoldCard)) ; }
	void onGameBegin()override{ stSitableRoomPlayer::onGameBegin(); memset(vHoldCard,0,sizeof(vHoldCard)) ; nCurAct = eRoomPeerAction_None ; nBetCoinThisRound = 0 ;}
};

class CTaxasPokerData
	:public CSitableRoomData
{
public:
	void onGameBegin()override;
	void setTaxasPlayerData(stTaxasPeerBaseData* pdata );
	bool onMsg(stMsg* pmsg );
	uint32_t getPlayerAddCoinLowLimit( uint8_t nPlayerSvrIdx );
	uint64_t getPlayerAddCoinUpLimit( uint8_t nPlayerSvrIdx);
	void resetBetRoundState();
	stSitableRoomPlayer* doCreateSitDownPlayer()override { return new stTaxasPlayer ;}
	uint8_t getRoomType()override{return eRoom_TexasPoker ;}
	uint8_t getTargetSvrPort()override{ return ID_MSG_PORT_TAXAS ;}
	CRobotControl* doCreateRobotControl();
public:
	uint32_t nLittleBlind;
	uint32_t nMiniTakeIn ;
	uint64_t nMaxTakeIn ;
	// running members ;
	int8_t nCurWaitPlayerActionIdx ;
	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;
	uint8_t vPublicCardNums[TAXAS_PUBLIC_CARD] ;

	// 
	bool bRobotWin ;
};
