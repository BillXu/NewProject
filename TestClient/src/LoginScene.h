#pragma once 
#include "IScene.h"
#include <string>
class CClientRobot;
class CLoginScene
	:public IScene
{
	enum eLoginState
	{
		els_Normal,
		els_WaitCoin,
		els_WaitEnterRoom ,
		els_WaitLogin,
		els_Max,
	};
public:
	CLoginScene(CClientRobot* pNetWork );
	virtual bool OnMessage( Packet* pPacket ) ;
	void Login( const char* pAccound , const char* pPassword );
	virtual void OnEnterScene();
	virtual void OnEixtScene();
	void OnUpdate(float fDeltaTime )override ;
	void CLoginScene::Verifyed();
	void InformIdle(unsigned int nUID = 0 );
	void CLoginScene::Register( const char* pName ,const char* pAccound , const char* pPassword , int nType );
	void doEnterGame();
	void delayEnterRoom();
	void delayLogin();
protected:
	float m_fDelyTick ;
	eLoginState m_eCurState ;
};