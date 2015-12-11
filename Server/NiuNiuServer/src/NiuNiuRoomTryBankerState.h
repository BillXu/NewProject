#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomTryBanker
	:public IRoomState
{
public:
	enum  
	{
		eStateID = 0x3,
	};
public:
	void enterState(IRoom* pRoom);
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	uint16_t getStateID(){ return eStateID ;}
	void onStateDuringTimeUp();
protected:
	CNiuNiuRoom* m_pRoom ;
	uint8_t m_nBiggestTimeTryBanker ;
};