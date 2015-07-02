#pragma once
#include "CommonDefine.h"
#include "cocos2d.h"
struct stMsg;
class CTaxasPlayer;
class CTaxasPokerScene;
class CTaxasPokerSceneStateBase
	:public cocos2d::Node
{
public:
	virtual bool init(CTaxasPokerScene* pScene);
	virtual void enterState(stMsg* pmsg){}
	virtual void leaveState(){}
	virtual bool onMsg(stMsg* pmsg );
	eRoomState getState(){ return m_eState ; }
protected:
	CTaxasPokerScene* m_pScene ;
	eRoomState m_eState;
};

// wait player join state 
class CTaxasPokerWaitJoinState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	bool onMsg(stMsg* pmsg );
};

// bet blind bet 
class CTaxasPokerBlindBetState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	bool onMsg(stMsg* pmsg );
};

// private card state 
class  CTaxasPokerPrivateCardState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	bool onMsg(stMsg* pmsg );

};

// betting state 
class CTaxasPokerBettingState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	bool onMsg(stMsg* pmsg );
	void onLocalPlayerActCallBack(CTaxasPlayer* pPlayer ,uint8_t nActValue, uint32_t nValue );
};

// one bet round end result 
class CTaxasPokerOneBetRoundEndResultState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	bool onMsg(stMsg* pmsg );
};	

// public card state
class CTaxasPokerPublicCardState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	void leaveState();
	bool onMsg(stMsg* pmsg );
protected:
	uint8_t nPublicRound;  // 0 init state , 1 distribute 3 card , 2 distribute 1 card , 3 d 1 card ;
};

// game result state
class CTaxasPokerGameResultState
	:public CTaxasPokerSceneStateBase
{
public:
	bool init(CTaxasPokerScene* pScene);
	void enterState(stMsg* pmsg) ;
	bool onMsg(stMsg* pmsg );
	void leaveState();
};
