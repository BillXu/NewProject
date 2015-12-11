#pragma once
#include "IRoomPlayer.h"
class ISitableRoomPlayer
	:public IRoomPlayer
{
public:
	uint8_t getIdx(){return m_nIdx ;}
	uint8_t setIdx( uint8_t nIdx){ m_nIdx = nIdx ;}
	virtual void doSitdown(uint8_t nIdx ){}
	virtual void willStandUp(){}
	bool isHaveState( uint32_t nStateFlag ){ return ( m_nState & nStateFlag ) == nStateFlag ;}
	void removeState( uint32_t nStateFlag ) { m_nState &=(~nStateFlag); }
	void addState( uint32_t nStateFlag ){ m_nState |=(nStateFlag); }
	void setState( uint32_t nStateFlag ){ m_nState = nStateFlag ;}
private:
	uint8_t m_nIdx ;
	uint32_t m_nState ;
};