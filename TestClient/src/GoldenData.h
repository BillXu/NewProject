#pragma once
#include "SitableRoomData.h"
#include <string>
#include "GoldenMessageDefine.h"
struct stGoldenPlayer
	:public stSitableRoomPlayer
{
	uint8_t vHoldChard[GOLDEN_PEER_CARD] ;

	void reset()override { stSitableRoomPlayer::reset();  memset(vHoldChard,0,sizeof(vHoldChard)) ;}
	void onGameBegin()override{ stSitableRoomPlayer::onGameBegin();nStateFlag = eRoomPeer_CanAct ;  memset(vHoldChard,0,sizeof(vHoldChard)) ; }
};

class CGoldenData
	:public CSitableRoomData
{
public:
	bool onMsg(stMsg* pmsg )override;
	stSitableRoomPlayer* doCreateSitDownPlayer()override ;
	uint8_t getRoomType()override ;
	uint8_t getTargetSvrPort()override ;
	CRobotControl* doCreateRobotControl()override ;
	void onGameBegin()override;
	uint16_t getRound(){ return nRound; }
	uint32_t getCurBet(){ return nCurBottomBet ; }
	uint32_t getBaseBet(){ return nBaseBet ;}
protected:
	uint16_t nRound ;
	uint32_t nCurBottomBet ;
	uint16_t nBaseBet ;
	uint8_t nBankerIdx ;
	uint8_t nPreActIdx ;
};