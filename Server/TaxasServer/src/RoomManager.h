#pragma once
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
class CTaxasRoom ;
class CRoomManager
{
public:
	typedef std::map<uint32_t, CTaxasRoom*> MAP_ID_ROOM;
public:
	CRoomManager();
	~CRoomManager();
	bool Init();
	bool OnMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	bool OnMsgFromOtherSvr( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nRoomID );
	CTaxasRoom* GetRoomByID(uint32_t nRoomID );
	void SendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID );
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
protected:
	MAP_ID_ROOM m_vRooms ;
};