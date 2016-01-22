#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
#include "CommonDefine.h"
class IRoom ;
struct stMsg ;
class IRoomState
{
public:
	IRoomState(){ m_fStateDuring = 0 ;}
	virtual ~IRoomState(){}
	virtual void enterState(IRoom* pRoom) = 0 ;
	virtual void leaveState(){}
	virtual void update(float fDeta);
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ){ return false ;}
	virtual uint16_t getStateID() = 0 ;
	virtual void onStateDuringTimeUp(){}
	void setStateDuringTime( float fTime ){ m_fStateDuring = fTime ;} 
	float getStateDuring(){ return m_fStateDuring ;}
private:
	float m_fStateDuring ;
};

class IRoomStateDead
	:public IRoomState
{
public:
	enum { eStateID = eRoomState_Dead };
public:
	IRoomStateDead(){ m_fStateDuring = 0 ; m_fMatchRestarTime = 0 ; m_MatchRoomDuringTime = 0 ;}
	virtual ~IRoomStateDead(){}
	virtual void enterState(IRoom* pRoom) ;
	virtual void leaveState(){}
	virtual void update(float fDeta);
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ){ return false ;}
	virtual uint16_t getStateID(){ return eStateID ;};
	virtual void onStateDuringTimeUp(){}
	void setStateDuringTime( float fTime ){ m_fStateDuring = fTime ;} 
	float getStateDuring(){ return m_fStateDuring ;}
private:
	float m_fStateDuring ;
	IRoom* m_pRoom ;

	float m_fMatchRestarTime ;
	uint32_t m_MatchRoomDuringTime ;
};