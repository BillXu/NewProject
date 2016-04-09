#pragma once
#include "CommonDefine.h"
#include "ISitableRoomPlayer.h"
#include "PeerCard.h"
#include "CommonData.h"
class CGoldenRoomPlayer
	:public ISitableRoomPlayer
{
public:
	void reset(IRoom::stStandPlayer* pPlayer) override ;
	void onGameEnd()override ;
	void onGameBegin()override ;
	void onGetCard( uint8_t nIdx , uint8_t nCard );
	CPeerCard* getPeerCard(){ return &m_tPeerCard ;}
	uint8_t getCardByIdx(uint8_t nIdx );
	void switchPeerCard(ISitableRoomPlayer* pPlayer )override;
	bool betCoin(uint32_t nBetCoin );
	uint32_t getBetCoin(){ return m_nBetCoin ;}
protected:
	uint32_t m_nBetCoin ;
	uint8_t m_vCards[GOLDEN_PEER_CARD];
	CPeerCard m_tPeerCard ;
};