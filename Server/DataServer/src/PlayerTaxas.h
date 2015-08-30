#pragma once
#include "IPlayerComponent.h"
#include <set>
class CPlayerTaxas
	:public IPlayerComponent
{
public:
	typedef std::set<uint32_t> SET_ROOM_ID ;
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
	uint32_t m_nCurTaxasRoomID ;
	bool m_bDirty ;
	stPlayerTaxasData m_tData ;
	SET_ROOM_ID m_vMyOwnRooms;
	SET_ROOM_ID m_vFollowedRooms ;
};