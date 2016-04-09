#pragma once
#include "IRoomManager.h"
class CGoldenRoomManager
	:public IRoomManager
{
public:
	CGoldenRoomManager(CRoomConfigMgr* pCongig ):IRoomManager(pCongig){ }
	eRoomType getMgrRoomType()override{ return eRoom_Golden ;}
protected:
	IRoomInterface* doCreateInitedRoomObject(uint32_t nRoomID ,bool isPrivateRoom, uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue)override ;
	IRoomInterface* doCreateRoomObject(eRoomType reqSubRoomType,bool isPrivateRoom)override ;
};