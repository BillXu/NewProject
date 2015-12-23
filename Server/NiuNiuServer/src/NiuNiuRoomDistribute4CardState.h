#pragma once
#include "IRoomState.h"
class ISitableRoom ;
class CNiuNiuRoomDistribute4CardState
	:public IRoomState
{
public:
	enum  
	{
		eStateID = eRoomState_NN_Disribute4Card ,
	};
public:
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp() ;
	uint16_t getStateID(){ return eStateID ;}
protected:
	ISitableRoom* m_pRoom ;
};