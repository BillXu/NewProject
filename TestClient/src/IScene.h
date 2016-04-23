#pragma once
#include "NetWorkManager.h"
class CClientRobot ;
enum eSceneType
{
	eScene_None,
	eScene_Login,
	eScene_Lab,
	eScene_Room,
	eScene_PaiJiuRoom,
	eScene_TaxasPoker,
	eScene_Bac,
	eScene_Max,
};

class IScene
	:public CNetMessageDelegate
{
public:
	IScene( CClientRobot* pClient ):m_eSceneType(eScene_None),m_pClient(pClient),m_bRunning(false){}
	virtual ~IScene(){}
	virtual void OnEnterScene();
	virtual void OnEixtScene();
	virtual bool OnMessage( Packet* pPacket ) ;
	virtual bool OnLostSever(Packet* pPacket );
	virtual void OnUpdate(float fDeltaTime ){};
	eSceneType GetSceneType(){return m_eSceneType ; }
	virtual bool OnConnectStateChanged( eConnectState eSate, Packet* pMsg );
	bool IsRunning(){ return m_bRunning ;}
	void SendMsg(char* pBuffer , unsigned short nLen ) ;
	void SendMsg(stMsg* pmsg , unsigned short nLen ){ SendMsg((char*)pmsg,nLen) ;}
	virtual void Verifyed(){};
	CClientRobot* getClient(){ return m_pClient ;}
protected:
	eSceneType m_eSceneType ;
	CClientRobot* m_pClient ;
	bool m_bRunning ;
};