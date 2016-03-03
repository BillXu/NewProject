#pragma once
#include "ISitableRoomManager.h"
class CNiuNiuRoomManager
	:public ISitableRoomManager
{
public:
	bool onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override;
	void sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )override;
	eRoomType getMgrRoomType()override{ return eRoom_NiuNiu ;}
protected:
	IRoom* doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue)override ;
	IRoom* doCreateRoomObject(eRoomType reqSubRoomType)override ;
};