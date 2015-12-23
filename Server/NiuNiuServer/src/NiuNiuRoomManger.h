#pragma once
#include "IRoomManager.h"
class CNiuNiuRoomManager
	:public IRoomManager
{
public:
	bool init()override;
	bool onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override;
	void sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )override;
protected:
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override;
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override;
	void onConnectedToSvr()override;
	IRoom* doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID )override ;
	void onGetChatRoomIDResult(IRoom* pNewRoom, bool bSuccess )override;
};