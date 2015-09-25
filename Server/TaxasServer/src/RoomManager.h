#pragma once
#include "TaxasMessageDefine.h"
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
#include "httpRequest.h"
#include <list>
class CTaxasRoom ;
class CRoomManager
	:public CHttpRequestDelegate
{
public:
	typedef std::list<CTaxasRoom*> LIST_ROOM ;
	typedef std::map<uint32_t, CTaxasRoom*> MAP_ID_ROOM;
	struct stRoomCreatorInfo
	{
		uint32_t nPlayerUID ;
		LIST_ROOM vRooms ;
	};
	typedef std::map<uint32_t,stRoomCreatorInfo> MAP_UID_CR;
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
	void onPlayerChangeRoom(uint32_t nCurRoomID , uint32_t nPlayerSessionID );
protected:
	bool reqeustChatRoomID(CTaxasRoom* pRoom);
	void addRoomToCreator(CTaxasRoom* pRoom);
	bool getRoomCreatorRooms(uint32_t nCreatorUID,LIST_ROOM& vInfo );
protected:
	MAP_ID_ROOM m_vRooms ;
	CHttpRequest m_pGoTyeAPI;
	uint32_t m_nMaxRoomID ;
	MAP_UID_CR m_vCreatorAndRooms ;
};