#pragma once
#include "NiuNiuMessageDefine.h"
#include "SitableRoomData.h"
#include <string>
struct stNiuNiuPlayer
	:public stSitableRoomPlayer
{
	uint8_t nBetTimes ;
	uint8_t vHoldChard[NIUNIU_HOLD_CARD_COUNT] ;

	void reset()override { stSitableRoomPlayer::reset(); nBetTimes = 0 ; memset(vHoldChard,0,sizeof(vHoldChard)) ;}
	void onGameBegin()override{ stSitableRoomPlayer::onGameBegin();nStateFlag = eRoomPeer_CanAct ; nBetTimes = 0 ; memset(vHoldChard,0,sizeof(vHoldChard)) ; }
};

class stNiuNiuData
	:public CSitableRoomData
{
public:
	stNiuNiuData();
	uint32_t getBaseBet(){ return nBottomBet ;}
	uint32_t getFinalBaseBet();
	void onGameBegin() override ;
	uint8_t getSitDownPlayerCnt();
	uint32_t getLeftCanBetCoin();
	bool onMsg(stMsg* pmsg);
	bool isHaveNiu(uint8_t nIdx );
	void resetAllStandupPlayer();
	stSitableRoomPlayer* doCreateSitDownPlayer()override { return new stNiuNiuPlayer ;}
	uint8_t getTargetSvrPort(){ return ID_MSG_PORT_NIU_NIU ;}
	uint8_t getRoomType(){ return eRoom_NiuNiu ;}
public:
	uint8_t nBankerIdx ;
	uint32_t nBottomBet ;
	uint8_t nBankerBetTimes ;
};