#pragma once
#include "CommonDefine.h"
#include "ISitableRoomPlayer.h"
#include "NiuNiuPeerCard.h"
#include "CommonData.h"
class CNiuNiuRoomPlayer
	:public ISitableRoomPlayer
{
public:
	void reset(IRoom::stStandPlayer* pPlayer) override ;
	void onGameEnd()override ;
	void onGameBegin()override ;
	void doSitdown(uint8_t nIdx ) override;
	void willStandUp() override;
	void onGetCard( uint8_t nIdx , uint8_t nCard );
	CNiuNiuPeerCard* getPeerCard(){ return &m_tPeerCard ;}
	uint8_t getCardByIdx(uint8_t nIdx );
	uint8_t getTryBankerTimes();
	void setTryBankerTimes(uint8_t nTimes );
	uint8_t getBetTimes();
	void setBetTimes(uint8_t nTimes);
	void switchPeerCard(ISitableRoomPlayer* pPlayer )override;
protected:
	uint8_t m_nTryBankerTimes ;
	uint8_t m_nBetTimes ;
	uint8_t m_vCards[NIUNIU_HOLD_CARD_COUNT];
	CNiuNiuPeerCard m_tPeerCard ;
};