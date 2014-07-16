#pragma once 
#include "IScene.h"
#include "CommonDefine.h"
class CClient;
class CPaiJiuScene
	:public IScene
{
public:
	enum eTargetAction
	{
		eTargetAction_None,
		eTargetAction_Bet,
		eTargetAction_Max,
	};
public:
	CPaiJiuScene(CClient* pClient);
	virtual void OnEnterScene();
	virtual bool OnMessage( RakNet::Packet* pPacket );
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
};