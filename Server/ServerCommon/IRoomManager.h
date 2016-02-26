#pragma once
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
#include "httpRequest.h"
#include <list>
class IRoom ;
class IRoomManager
	:public CHttpRequestDelegate
{
public:
	typedef std::list<IRoom*> LIST_ROOM ;
	typedef std::map<uint32_t, IRoom*> MAP_ID_ROOM;
	typedef std::map<uint32_t,LIST_ROOM> MAP_CONFIG_ROOMS ;
	struct stRoomCreatorInfo
	{
		uint32_t nPlayerUID ;
		LIST_ROOM vRooms ;
	};
	typedef std::map<uint32_t,stRoomCreatorInfo> MAP_UID_CR;
public:
	IRoomManager();
	~IRoomManager();
	virtual bool init();
	bool onMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	virtual bool onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID);
	IRoom* GetRoomByID(uint32_t nRoomID );
	virtual IRoom* getRoomByConfigID(uint32_t nRoomID ) = 0 ;
	virtual void sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID ) = 0 ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg);
	virtual void update(float fDelta );
	virtual void onTimeSave();
protected:
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	virtual void onConnectedToSvr();
	virtual IRoom* doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID,eRoomType cRoomType, Json::Value& vJsValue ) = 0 ;
	virtual IRoom* doCreateRoomObject( eRoomType cRoomType) = 0 ;
	bool reqeustChatRoomID(IRoom* pRoom);
	void addRoomToCreator(IRoom* pRoom);
	void addRoomToConfigRooms(IRoom* pRoom);
	bool getRoomCreatorRooms(uint32_t nCreatorUID,LIST_ROOM& vInfo );
	void removeRoom(IRoom* pRoom );
	void doDeleteRoom(IRoom* pRoom );
protected:
	MAP_ID_ROOM m_vRooms ;
	MAP_CONFIG_ROOMS m_vCongfigIDRooms ;
	CHttpRequest m_pGoTyeAPI;
	uint32_t m_nMaxRoomID ;
	MAP_UID_CR m_vCreatorAndRooms ;
};