#pragma once
#include "ISitableRoomPlayer.h"
#include "TaxasPokerPeerCard.h"
class CTaxasPlayer
	:public ISitableRoomPlayer
{
public:
	void willStandUp()override ;
	void onGameEnd() override;
	void onGameBegin()override ;
	void reset(IRoom::stStandPlayer* pPlayer)override;
	void switchPeerCard(ISitableRoomPlayer* pPlayer )override;
	uint8_t getPeerCardByIdx(uint8_t nIdx );
	void addPublicCard(uint8_t vPublicCards[TAXAS_PUBLIC_CARD] );
	void addPeerCard(uint8_t nIdx , uint8_t nCardNum );
	bool betCoin( uint32_t nBetCoin );
	CTaxasPokerPeerCard* getPeerCard(){ return &m_tPeerCard ;}
	eRoomPeerAction getCurActType();
	uint32_t getWinCoinThisGame();
	void addWinCoinThisGame(uint32_t nWinCoin );
	int32_t getCoinOffsetThisGame();
	uint32_t getBetCoinThisRound(){ return nBetCoinThisRound ; }
	void setBetCoinThisRound( int32_t nValue ){ nBetCoinThisRound = nValue ;}
	void setCurActType(eRoomPeerAction act){ eCurAct = act ;};
	uint32_t getAllBetCoin(){ return nAllBetCoin ;}
protected:
	void zeroData();
protected:
	uint8_t vPeerCards[TAXAS_PEER_CARD];
	eRoomPeerAction eCurAct ;  
	uint32_t nBetCoinThisRound ;
	uint32_t nAllBetCoin ;  // used for tell win or lose
	uint32_t nWinCoinThisGame ;    // used for tell win or lose
	CTaxasPokerPeerCard m_tPeerCard ;
};