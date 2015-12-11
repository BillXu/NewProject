#pragma once
#include "CommonDefine.h"
#include "ISitableRoomPlayer.h"
#include "NiuNiuPeerCard.h"
class CNiuNiuRoomPlayer
	:public ISitableRoomPlayer
{
public:
	void init(uint32_t nSessionID , uint32_t nUserUID) override;
	void willLeave()override ;
	void onGameEnd()override ;
	void onGameBegin()override ;
	void doSitdown(uint8_t nIdx ) override;
	void willStandUp() override;
	void onGetCard( uint8_t nIdx , uint8_t nCard );
	uint8_t getTryBankerTimes();
	CNiuNiuPeerCard* getPeerCard(){ return &m_tPeerCard ;}
protected:
	uint8_t m_nTryBankerTimes ;
	uint8_t m_nBetTimes ;
	uint8_t m_vCards[NIUNIU_HOLD_CARD_COUNT];
	uint64_t m_nCoin ;
	CNiuNiuPeerCard m_tPeerCard ;
};