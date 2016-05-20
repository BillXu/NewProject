#pragma once
#include "ServerMessageDefine.h"
#include <map>
#include <json/json.h>
#include "httpRequest.h"
#include "IGlobalModule.h"
#include <list>
class CRoomConfigMgr ;
class IRoomInterface ;
class IRoom ;

struct stPrivateRoomRecorder
{
	uint32_t nRoomID ;
	uint32_t nCreaterUID ;
	uint16_t nConfigID ;
	time_t nTime ;
	uint32_t nDuringSeconds ;
	Json::Value playerDetail ;
};

class IRoomManager
	:public CHttpRequestDelegate
	,public IGlobalModule
{
public:
	typedef std::list<IRoomInterface*> LIST_ROOM ;
	typedef std::map<uint32_t, IRoomInterface*> MAP_ID_ROOM;
	struct stRoomCreatorInfo
	{
		uint32_t nPlayerUID ;
		LIST_ROOM vRooms ;
	};
	typedef std::map<uint32_t,stRoomCreatorInfo> MAP_UID_CR;
public:
	IRoomManager(CRoomConfigMgr* pConfigMgr);
	~IRoomManager();
	void init( IServerApp* svrApp )override; 
	uint16_t getModuleType(){ return IGlobalModule::eMod_RoomMgr ;}
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override;
	virtual bool onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID);
	IRoomInterface* GetRoomByID(uint32_t nRoomID );
	void sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID ) ;
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg);
	void update(float fDeta )override;
	void onTimeSave()override;
	void onConnectedSvr()override;
	bool reqeustChatRoomID(IRoom* pRoom);
	void deleteRoomChatID(uint32_t nChatID );
	void addPrivateRoomRecorder( stPrivateRoomRecorder* pRecorder, bool isSaveDB = true );
protected:
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	virtual IRoomInterface* doCreateRoomObject( eRoomType cRoomType,bool isPrivateRoom ) = 0 ;
	virtual IRoomInterface* doCreateInitedRoomObject(uint32_t nRoomID , bool isPrivateRoom, uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue) = 0 ;
	void addRoomToCreator(uint32_t nOwnerUID ,IRoomInterface* pRoom);
	bool getRoomCreatorRooms(uint32_t nCreatorUID,LIST_ROOM& vInfo );
	void removeRoom(IRoomInterface* pRoom );
	virtual eRoomType getMgrRoomType() = 0 ;
protected:
	MAP_ID_ROOM m_vRooms ;

	CHttpRequest m_pGoTyeAPI;
	uint32_t m_nMaxRoomID ;
	MAP_UID_CR m_vCreatorAndRooms ;
	CRoomConfigMgr* m_pConfigMgr ;

	std::map<uint32_t,stPrivateRoomRecorder*> m_mapPrivateRecorder ;
};