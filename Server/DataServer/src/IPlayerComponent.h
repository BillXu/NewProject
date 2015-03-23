#pragma once
#include "MessageDefine.h"
class CPlayer ;

enum ePlayerComponentType
{
	ePlayerComponent_None ,
	ePlayerComponent_BaseData,
	ePlayerComponent_Friend,
	ePlayerComponent_PlayerShop,
	ePlayerComponent_PlayerItemMgr,
	ePlayerComponent_PlayerMission,
	ePlayerComponent_Mail,            // last sit the last pos ,
	ePlayerComponent_Max,
};

struct stPlayerEvetArg ;
class IPlayerComponent
{
public:
	IPlayerComponent(CPlayer* pPlayer );
	virtual ~IPlayerComponent();
	void SendMsg(stMsg* pbuffer , unsigned short nLen , bool bBrocast = false );
	ePlayerComponentType GetComponentType(){ return m_eType ;}
	CPlayer* GetPlayer(){ return m_pPlayer ;}
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort) = 0 ;
	virtual void OnPlayerDisconnect(){}
	virtual void Reset(){}
	virtual void Init(){ }
	virtual void OnOtherWillLogined(){}
	virtual void OnOtherDoLogined(){}
	virtual bool OnPlayerEvent(stPlayerEvetArg* pArg){ return false ; }
	virtual void TimerSave(){};
	virtual void OnReactive(uint32_t nSessionID ){ }
protected:
	CPlayer* m_pPlayer ;
	ePlayerComponentType m_eType ;	
};