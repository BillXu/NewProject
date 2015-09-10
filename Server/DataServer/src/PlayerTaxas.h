#pragma once
#include "IPlayerComponent.h"
#include <set>
class CPlayerTaxas
	:public IPlayerComponent
{
public:
	typedef std::set<uint32_t> SET_ROOM_ID ;
	typedef std::map<uint32_t,stMyOwnRoom> MAP_ID_MYROOW ;
public:
	CPlayerTaxas(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_eType = ePlayerComponent_PlayerTaxas ; }
	virtual void Reset();
	virtual void Init();
	virtual bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort) ;
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	virtual void OnPlayerDisconnect();
	virtual void OnOtherWillLogined();
	virtual void TimerSave();
protected:
	bool isRoomIDMyOwn(uint32_t nRoomID ){ return true ;}
protected:
	uint32_t m_nCurTaxasRoomID ;
	bool m_bDirty ;
	stPlayerTaxasData m_tData ;
	MAP_ID_MYROOW m_vMyOwnRooms;
	SET_ROOM_ID m_vFollowedRooms ;
};