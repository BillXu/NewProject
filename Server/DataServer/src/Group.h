#pragma once
#include "IGlobalModule.h"
#include <algorithm>
#include "httpRequest.h"
struct stGroupItem
{
	uint32_t nCreaterUID ;
	uint32_t nCityCode ;
	uint32_t nGroupID ;
	uint32_t nCurCnt ;
	uint16_t nLevel ;
	bool isDirty ;
	bool isCntDirty ;
	uint32_t m_tLevelRunOutTime ;
public:
	stGroupItem(){ nCurCnt = 1 ; nLevel = 0 ; m_tLevelRunOutTime = 0 ; isCntDirty = false ; isDirty = false ; }
	bool isRoomKeepRunning();
	bool isGroupFull();
	uint32_t getCapacity();
	bool isPlayerCanCreateRoom( uint32_t nUserUID ){ return nUserUID == nCreaterUID ;}
};

class CGroup
	:public IGlobalModule
	,public CHttpRequestDelegate
{
public:
	typedef std::map<uint32_t,stGroupItem*> MAP_GROUP ;
	enum eHttpReq 
	{
		eReq_AddMember ,
		eReq_DeleteMember,
		eReq_RefreshCnt,
		eReq_Max,
	};
public:
	~CGroup();
	uint16_t getModuleType()override{ return eMod_Group ;};
	void init( IServerApp* svrApp )override ;
	void onConnectedSvr()override ;
	bool onMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)override ;
	bool onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)override ;
	stGroupItem* getGroupByID(uint32_t nGroupID );
	void addGroup(stGroupItem* pItem );
	void dismissGroup(uint32_t nGroupID );
	uint16_t getClubCntByUserUID(uint32_t nUserUID);
	void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)override ;
	void onTimeSave()override ;
protected:
	MAP_GROUP m_vGroups ;
	CHttpRequest m_pGoTyeAPI;
};