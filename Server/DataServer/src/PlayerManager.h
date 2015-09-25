#pragma once 
#include "NetWorkManager.h"
#include "ServerMessageDefine.h"
#include <json/json.h>
#include <map>
#include <list>
class CPlayer ;
struct stMsg ;
class CSelectPlayerDataCacher
{
public:
	typedef std::map<uint32_t,stPlayerBrifData*> MAP_ID_DATA;
public:
	CSelectPlayerDataCacher();
	~CSelectPlayerDataCacher();
	void removePlayerDataCache( uint32_t nUID );
	void cachePlayerData(stMsgSelectPlayerDataRet* pmsg );
	bool getPlayerData(uint32_t nUID , stPlayerBrifData* pData , bool isDetail );
protected:
	MAP_ID_DATA m_vBrifData ;
	MAP_ID_DATA m_vDetailData ;
};
class CPlayerManager
{
public:
	typedef std::map<uint32_t, CPlayer*> MAP_SESSIONID_PLAYERS ; 
	typedef std::map<uint32_t, CPlayer*> MAP_UID_PLAYERS ;
	typedef std::list<CPlayer*> LIST_PLAYERS ;
public:
	CPlayerManager();
	~CPlayerManager();
	bool OnMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID  );
	CPlayer* GetPlayerByUserUID( uint32_t nUserUID, bool bInclueOffline = true );
	CPlayer* GetPlayerBySessionID(uint32_t nSessionID , bool bInclueOffline = false );
	void Update(float fDeta );
	CPlayer* GetFirstActivePlayer();
protected:
	void OnPlayerOffline(CPlayer* pOfflinePlayer);
	bool ProcessPublicMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr );
	void AddPlayer(CPlayer*);
	void LogState();
	bool ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID ) ;
public:
	CSelectPlayerDataCacher& getPlayerDataCaher(){ return m_tPlayerDataCaher ;}
protected:
	// logic data ;
	MAP_SESSIONID_PLAYERS m_vAllActivePlayers ;
	MAP_UID_PLAYERS m_vOfflinePlayers ;

	CSelectPlayerDataCacher m_tPlayerDataCaher ;
};