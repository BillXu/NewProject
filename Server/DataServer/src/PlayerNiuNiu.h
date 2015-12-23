#pragma once
#include "IPlayerComponent.h"
class CPlayerNiuNiu
	:public IPlayerComponent
{
public:
	CPlayerNiuNiu(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_eType = ePlayerComponent_PlayerNiuNiu ; }
	void Reset()override;
	void Init()override;
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort)override ;
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override;
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override;
	void OnPlayerDisconnect()override;
	void OnOtherWillLogined()override;
	void TimerSave();
	void OnReactive(uint32_t nSessionID )override;
	void OnOtherDoLogined();
	uint32_t getCurRoomID(){ return m_nCurRoomID ;}
protected:
	uint16_t m_nCurRoomID ;
};