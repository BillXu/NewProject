#pragma once
#include "MessageDefine.h"
class IServerApp ;
class IGlobalModule
{
public:
	enum  eModule
	{
		eMod_None,
		eMod_RoomMgr,
		eMod_Max
	}; 
public:
	IGlobalModule(){ m_fTicket = 300; m_app = nullptr ;}
	virtual ~IGlobalModule(){}
	virtual uint16_t getModuleType() = 0 ;
	IServerApp* getSvrApp(){ return m_app; }
	virtual void init( IServerApp* svrApp ) { m_app = svrApp ;}
	virtual void onExit(){ onTimeSave() ;}
	virtual bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID){ return false ;}
	virtual void update(float fDeta )
	{
		m_fTicket -= fDeta ;
		if ( m_fTicket < 0 )
		{
			m_fTicket = getTimeSave();
			onTimeSave() ;
		}
	}
	virtual void onTimeSave(){}
	virtual void onConnectedSvr(){}
	virtual float getTimeSave(){ return 650; }
private:
	IServerApp* m_app ;
	float m_fTicket ;
};