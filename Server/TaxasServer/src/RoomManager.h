#pragma once
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
#include "httpRequest.h"
class CTaxasRoom ;
class CRoomManager
	:public CHttpRequestDelegate
{
public:
	typedef std::map<uint32_t, CTaxasRoom*> MAP_ID_ROOM;
public:
	CRoomManager();
	~CRoomManager();
	bool Init();
	bool OnMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	bool OnMsgFromOtherSvr( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nRoomID );
	bool onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID);
	CTaxasRoom* GetRoomByID(uint32_t nRoomID );
	void SendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID );
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg);
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	void onConnectedToSvr();
protected:
	bool reqeustChatRoomID(CTaxasRoom* pRoom);
protected:
	MAP_ID_ROOM m_vRooms ;
	CHttpRequest m_pGoTyeAPI;
	uint32_t m_nMaxRoomID ;
};