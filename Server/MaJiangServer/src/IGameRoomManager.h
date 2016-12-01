#pragma once
#include "IGameRoom.h"
#include "IGlobalModule.h"
#include <json/json.h>
#include "IGameRecorder.h"
struct stMsg;
class IGameRoomManager
	:public IGlobalModule
{
public:
	virtual ~IGameRoomManager(){}
	virtual IGameRoom* getRoomByID(uint32_t nRoomID) = 0;
	virtual void sendMsg(stMsg* pmsg, uint32_t nLen, uint32_t nSessionID) = 0;
	virtual void sendMsg(Json::Value& jsContent, unsigned short nMsgType, uint32_t nSessionID, eMsgPort ePort = ID_MSG_PORT_CLIENT) = 0;
	virtual void addGameRecorder(IGameRecorder::shared_ptr& pBill, bool isAddtoDB) = 0;
	virtual bool isHaveGameRecorder(uint32_t nRoomID) = 0;
	virtual void sendGameRecorderToPlayer(uint32_t nRoomID, uint32_t nTargetSessionD) = 0;
};