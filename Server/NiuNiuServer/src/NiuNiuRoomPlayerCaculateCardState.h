#pragma once 
#include "IRoomState.h"
#include <map>
class ISitableRoom ;
class CNiuNiuRoomStatePlayerCaculateCardState
	:public IRoomState
{
public:
	enum  { eStateID = eRoomState_NN_CaculateCard };
	uint16_t getStateID(){ return eStateID ;}
	void enterState(IRoom* pRoom);
	void onStateDuringTimeUp();
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
protected:
	ISitableRoom* m_pRoom ;
	uint8_t m_nWaitOperPlayerCnt ;
	std::map<uint32_t,uint8_t> m_vOperoateRecord ;
};