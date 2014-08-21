#pragma once 
#include "IPlayerComponent.h"
#include "Timer.h"
class CPlayerBaseData ;
struct stMsg ;
class CRoomBaseNew ;
class CPlayer
	:public CTimerDelegate
{
public:
	enum ePlayerState
	{
		ePlayerState_Free,
		ePlayerState_InRoom,
		ePlayerState_WillLeavingRoom,
		ePlayerState_Max,
	};
public:
	CPlayer();
	~CPlayer();
	void Init( unsigned int nUserUID,unsigned int nSessionID );
	void Reset(unsigned int nUserUID,unsigned int nSessionID ) ; // for reuse the object ;
	void OnMessage(stMsg* pMsg );
	void OnPlayerDisconnect();
	void SendMsgToClient(const char* pBuffer, unsigned short nLen,bool bBrocat = false );
	void SendMsgToDBServer(const char* pBuffer, unsigned short nLen);
	unsigned int GetUserUID(){ return m_nUserUID ;}
	unsigned int GetSessionID(){ return m_nSessionID ;}
	IPlayerComponent* GetComponent(ePlayerComponentType eType ){ return m_vAllComponents[eType];}
	CPlayerBaseData* GetBaseData(){ return (CPlayerBaseData*)GetComponent(ePlayerComponent_BaseData);}
	ePlayerState GetState(){ return m_eSate ; } 
	void SetState(ePlayerState eSate ){ m_eSate = eSate ; }
	void OnAnotherClientLoginThisPeer(unsigned int nSessionID );
	void PostPlayerEvent(stPlayerEvetArg* pEventArg );
	CRoomBaseNew* GetRoomCurStateIn();
	void OnTimerSave(float fTimeElaps,unsigned int nTimerID );
protected:
	bool ProcessPublicPlayerMsg(stMsg* pMsg);
	void PushTestAPNs();
protected:
	unsigned int m_nUserUID ;
	unsigned int m_nSessionID ;  // comunicate with the client ;
	IPlayerComponent* m_vAllComponents[ePlayerComponent_Max] ;
	ePlayerState m_eSate ;
	CTimer* m_pTimerSave ;
	bool m_bDisconnected ;
	CRoomBaseNew* m_pCurRoom ;
}; 