#pragma once 
#include "IScene.h"
#include <string>
class CClientRobot;
class CLoginScene
	:public IScene
{
public:
	CLoginScene(CClientRobot* pNetWork );
	virtual bool OnMessage( Packet* pPacket ) ;
	void Login( const char* pAccound , const char* pPassword );
	virtual void OnEnterScene();
	virtual void OnEixtScene();
	void CLoginScene::Verifyed();
	void InformIdle();
	void CLoginScene::Register( const char* pName ,const char* pAccound , const char* pPassword , int nType );
};