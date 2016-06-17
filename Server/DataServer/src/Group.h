#pragma once
#include "IGlobalModule.h"
#include <algorithm>
struct stGroupItem
{
	uint32_t nCreaterUID ;
	uint32_t nCityCode ;
	uint32_t nGroupID ;
	std::vector<uint32_t> vAllRoomIDs ;
public:
	bool isRoomKeepRunning(){ return vAllRoomIDs.empty() == false ;}
	void addRoomID(uint32_t nRoomID ){ vAllRoomIDs.push_back(nRoomID) ; }
	void removeRoomID(uint32_t nRoomID ){ auto iter =  std::find(vAllRoomIDs.begin(),vAllRoomIDs.end(),nRoomID) ; if ( iter != vAllRoomIDs.end() )vAllRoomIDs.erase(iter) ;}
	bool isHaveRoomID(uint32_t nRoomID ){ return std::find(vAllRoomIDs.begin(),vAllRoomIDs.end(),nRoomID) != vAllRoomIDs.end() ;}
	bool isPlayerCanCreateRoom( uint32_t nUserUID ){ return nUserUID == nCreaterUID ;}
};

class CGroup
	:public IGlobalModule
{
public:
	typedef std::map<uint32_t,stGroupItem*> MAP_GROUP ;
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
protected:
	MAP_GROUP m_vGroups ;
};