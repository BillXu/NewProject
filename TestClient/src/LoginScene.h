#pragma once 
#include "IScene.h"
#include <string>
class CClient;
class CLoginScene
	:public IScene
{
public:
	CLoginScene(CClient* pNetWork );
	virtual bool OnMessage( RakNet::Packet* pPacket ) ;
	void Login( const char* pAccound , const char* pPassword );
	virtual void OnEnterScene();
	virtual void OnEixtScene();
	void CLoginScene::Verifyed();
	void InformIdle();
};