#pragma once
#include "IRoomState.h"
class CNiuNiuRoom ;
class CNiuNiuRoomRandBankerState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_NN_RandBanker ,
	};
public:
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp();
	uint16_t getStateID(){ return eStateID ;}
protected:
	CNiuNiuRoom* m_pRoom ;
};