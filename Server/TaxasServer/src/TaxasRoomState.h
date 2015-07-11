#pragma once
#include "TaxasMessageDefine.h"
class CTaxasRoom ;
class CTaxasBaseRoomState
{
public:
	CTaxasBaseRoomState();
	virtual ~CTaxasBaseRoomState();
	virtual bool OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	virtual void EnterState(CTaxasRoom* pRoom );
	virtual void LeaveState();
	virtual void Update(float fDelte );
	virtual void OnStateTimeOut(){}
	float GetDuringTime(){ return m_fDuringTime ;}
	void SetState(eRoomState eState ){ m_eState = eRoomState_TP_MAX ; }
protected:
	float m_fDuringTime ;
	CTaxasRoom* m_pRoom ;
	eRoomState m_eState ;
};

// wait join state 
class CTaxasStateWaitJoin
	:public CTaxasBaseRoomState
{
public:
	virtual void EnterState(CTaxasRoom* pRoom );
	void Update(float fDelte );
};

// start blind bet state 
class CTaxasStateBlindBet
	:public CTaxasBaseRoomState
{
public:
	void EnterState(CTaxasRoom* pRoom );
	void OnStateTimeOut();
};

// private card
class CTaxasStatePrivateCard
	:public CTaxasBaseRoomState
{
public:
	void EnterState(CTaxasRoom* pRoom );
	void OnStateTimeOut();
};

// player bet state 
class CTaxasStatePlayerBet
	:public CTaxasBaseRoomState
{
public:
	void EnterState(CTaxasRoom* pRoom );
	void OnStateTimeOut();
	void Update(float fDelte );
	bool OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	void ResetStateTime();
	void PlayerDoActOver();
protected:
	uint32_t m_nCurActPlayerIdx ;
	bool m_bHavePlayerActing ;
	float m_fLeftActingTime ;
	bool m_bIsCurActPlayerActing ;
};

// caculate vice pool
class CTaxasStateOneRoundBetEndResult
	:public CTaxasBaseRoomState
{
public:
	void EnterState(CTaxasRoom* pRoom );
	void OnStateTimeOut();
};

// public card
class CTaxasStatePublicCard
	:public CTaxasBaseRoomState
{
public:
	void EnterState(CTaxasRoom* pRoom );
	void OnStateTimeOut();
};

// game result 
class CTaxasStateGameResult
	:public CTaxasBaseRoomState
{
public:
	void EnterState(CTaxasRoom* pRoom );
	void OnStateTimeOut();
};