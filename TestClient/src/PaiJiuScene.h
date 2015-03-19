#pragma once 
#include "IScene.h"
#include "CommonDefine.h"
class CClientRobot;
class CPaiJiuScene
	:public IScene
{
public:
	enum eTargetAction
	{
		eTargetAction_None,
		eTargetAction_Bet,
		eTargetAction_ApplyBanker,
		eTargetAction_Max,
	};
public:
	CPaiJiuScene(CClientRobot* pClient);
	virtual void OnEnterScene();
	virtual bool OnMessage( Packet* pPacket );
	virtual void OnUpdate(float fDeltaTime );
protected:
	void TryToBeBanker();
	void BetCoin();
protected:
	eRoomState m_eRoomState ;
	bool m_bBanker ;
	unsigned short nApplyerCount ;
	unsigned char nRoundWhenBanker ;
	unsigned int nBetTimesThisRound ; 

	// timer ticket 
	eTargetAction m_eTargetAction ;
	float m_fTick ;
	float m_fTargetTick[eTargetAction_Max] ;
	bool m_bWillApplyBanker;
};