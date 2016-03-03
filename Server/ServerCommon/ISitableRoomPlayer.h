#pragma once
#include "ServerCommon.h"
#include "IRoom.h"
class ISitableRoomPlayer
{
public:
	uint8_t getIdx(){return m_nIdx ;}
	void setIdx( uint8_t nIdx){ m_nIdx = nIdx ;}
	virtual void doSitdown(uint8_t nIdx ){ m_nIdx = nIdx ;}
	virtual void willStandUp(){}
	virtual void onGameEnd() ;
	virtual void onGameBegin(){ ++nPlayTimes ;}
	bool isHaveState( uint32_t nStateFlag ){ return ( m_nState & nStateFlag ) == nStateFlag ;}
	void removeState( uint32_t nStateFlag ) { m_nState &=(~nStateFlag); }
	void addState( uint32_t nStateFlag ){ m_nState |=(nStateFlag); }
	void setState( uint32_t nStateFlag ){ m_nState = nStateFlag ;}
	uint32_t getState(){ return m_nState ; }
	uint64_t getCoin(){ return nCoin ;}
	void setCoin( int64_t nNewCoin ){ nCoin = nNewCoin ;}
	uint32_t getUserUID(){ return nUserUID ; }
	uint32_t getSessionID(){ return nSessionID ; }
	virtual void reset(IRoom::stStandPlayer* pPlayer);
	bool isHaveHalo();
	virtual void switchPeerCard(ISitableRoomPlayer* pPlayer ) = 0;
	uint8_t getHaloWeight(){ return nNewPlayerHaloWeight ; }
	bool isDelayStandUp(){ return m_isDelayStandUp ;}
	void delayStandUp(){ m_isDelayStandUp = true ; }
	uint32_t getWinTimes(){ return nWinTimes ; }
	uint32_t getPlayTimes(){ return nPlayTimes ;}
	uint64_t getSingleWinMost(){ return nSingleWinMost ;}
	void increaseWinTimes(){ ++nWinTimes ;}
private:
	bool m_isDelayStandUp;
	uint8_t m_nIdx ;
	uint32_t m_nState ;
	uint8_t m_nHaloState ; // 0 not invoke , 1 in effect , 2 not in effect ;

	uint64_t nCoin ;
	uint32_t nUserUID ;
	uint32_t nSessionID ;
	uint8_t nNewPlayerHaloWeight;
	uint32_t nPlayTimes ;
	uint32_t nWinTimes ;
	uint64_t nSingleWinMost ;
};