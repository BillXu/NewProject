#pragma once
#include "NetWorkManager.h"
class CClient ;
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
	IScene( CClient* pClient ):m_eSceneType(eScene_None),m_pClient(pClient),m_bServerConnected(false),m_bRunning(false){}
	virtual ~IScene(){}
	virtual void OnEnterScene();
	virtual void OnEixtScene();
	virtual bool OnMessage( RakNet::Packet* pPacket ) ;
	virtual bool OnLostSever(RakNet::Packet* pPacket );
	virtual void OnUpdate(float fDeltaTime ){};
	eSceneType GetSceneType(){return m_eSceneType ; }
	virtual bool OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg);
	bool IsRunning(){ return m_bRunning ;}
	void SendMsg(char* pBuffer , unsigned short nLen ) ;
	void SendMsg(stMsg* pmsg , unsigned short nLen ){ SendMsg((char*)pmsg,nLen) ;}
	virtual void Verifyed(){};
protected:
	eSceneType m_eSceneType ;
	CClient* m_pClient ;
	bool m_bServerConnected ;
	bool m_bRunning ;
};