#pragma once 
#include "NetWorkManager.h"
#include "MessageDefine.h"
#include <map>
#include <list>
class CPlayer ;
struct stMsg ;
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
	void AddPlayer(CPlayer*);
	void LogState();
	bool ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID ) ;
protected:
	// logic data ;
	MAP_SESSIONID_PLAYERS m_vAllActivePlayers ;
	MAP_UID_PLAYERS m_vOfflinePlayers ;
};