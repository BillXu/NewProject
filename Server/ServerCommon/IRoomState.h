#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
class IRoom ;
struct stMsg ;
class IRoomState
{
public:
	IRoomState(){ m_fStateDuring = 0 ;}
	virtual ~IRoomState();
	virtual void enterState(IRoom* pRoom) = 0 ;
	virtual void leaveState(){}
	virtual void update(float fDeta);
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ){ return false ;}
	virtual uint16_t getStateID() = 0 ;
	virtual void onStateDuringTimeUp(){}
	void setStateDuringTime( float fTime ){ m_fStateDuring = fTime ;} 
private:
	float m_fStateDuring ;
};