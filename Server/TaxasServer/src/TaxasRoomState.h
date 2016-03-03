#pragma once
#include "TaxasMessageDefine.h"
#include "IRoomState.h"
////class CTaxasRoom ;
////class CTaxasBaseRoomState
////{
////public:
////	CTaxasBaseRoomState();
////	virtual ~CTaxasBaseRoomState();
////	virtual bool OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
////	virtual void EnterState(CTaxasRoom* pRoom );
////	virtual void LeaveState();
////	virtual void Update(float fDelte );
////	virtual void OnStateTimeOut(){}
////	float GetDuringTime(){ return m_fDuringTime ;}
////	void SetState(eRoomState eState ){ m_eState = eRoomState_TP_MAX ; }
////protected:
////	float m_fDuringTime ;
////	CTaxasRoom* m_pRoom ;
////	eRoomState m_eState ;
////};
////
////// dead state 
////class CTaxasStateDead
////	:public CTaxasBaseRoomState
////{
////public:
////	CTaxasStateDead(){ m_fMatchRestarTime = 0 ; m_MatchRoomDuringTime = 0 ;}
////	virtual void EnterState(CTaxasRoom* pRoom );
////	void Update(float fDelte );
////protected:
////	float m_fMatchRestarTime ;
////	uint32_t m_MatchRoomDuringTime ;
////};
////
////// wait join state 
////class CTaxasStateWaitJoin
////	:public CTaxasBaseRoomState
////{
////public:
////	virtual void EnterState(CTaxasRoom* pRoom );
////	void Update(float fDelte );
////};
////
////// start blind bet state 
////class CTaxasStateBlindBet
////	:public CTaxasBaseRoomState
////{
////public:
////	void EnterState(CTaxasRoom* pRoom );
////	void OnStateTimeOut();
////};
class CTaxasRoom ;
// start game private card
class CTaxasStateStartGame
	:public IRoomState
{
public:
	CTaxasStateStartGame(){ m_nState = eRoomState_StartGame ; }
	void enterState(IRoom* pRoom)override;
	void onStateDuringTimeUp()override;
protected:
	CTaxasRoom* m_pRoom ;
};

// player bet state 
class CTaxasStatePlayerBet
	:public IRoomState
{
public:
	CTaxasStatePlayerBet(){ m_nState = eRoomState_TP_Beting ; }
	void enterState(IRoom* pRoom)override;
	void onStateDuringTimeUp()override;
	void update(float fDeta)override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override;
	void resetStateTime();
	void playerDoActOver();
protected:
	CTaxasRoom* m_pRoom ;
	bool m_bIsDoFinished ;
};

// caculate vice pool
class CTaxasStateOneRoundBetEndResult
	:public IRoomState
{
public:
	CTaxasStateOneRoundBetEndResult(){ m_nState = eRoomState_TP_OneRoundBetEndResult ; }
	void enterState(IRoom* pRoom)override;
	void onStateDuringTimeUp()override;
protected:
	CTaxasRoom* m_pRoom ;
};

// public card
class CTaxasStatePublicCard
	:public IRoomState
{
public:
	CTaxasStatePublicCard(){ m_nState = eRoomState_TP_PublicCard ; }
	void enterState(IRoom* pRoom)override;
	void onStateDuringTimeUp()override;
protected:
	CTaxasRoom* m_pRoom ;
};

// game result 
class CTaxasStateGameResult
	:public IRoomState
{
public:
	CTaxasStateGameResult(){ m_nState = eRoomState_TP_GameResult ; }
	void enterState(IRoom* pRoom)override;
	void onStateDuringTimeUp()override;
protected:
	CTaxasRoom* m_pRoom ;
};