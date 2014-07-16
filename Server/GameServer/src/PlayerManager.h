#pragma once 
#include "NetWorkManager.h"
#include <map>
#include <list>
class CPlayer ;
struct stMsg ;
class CPlayerManager
{
public:
	typedef std::map<unsigned int, CPlayer*> MAP_PLAYERS ;  // nsessionID , player ;
	typedef std::list<CPlayer*> LIST_PLAYERS ;
public:
	CPlayerManager();
	~CPlayerManager();
	bool OnMessage( RakNet::Packet* pMsg );
	CPlayer* GetPlayerByUserUID( unsigned int nUserUID, bool IgnorWillRemovePlayer = true );
	CPlayer* GetPlayerBySessionID(unsigned int nSessionID );
	void Update(float fDeta );
	CPlayer* GetFirstActivePlayer();
	void RemovePlayer(CPlayer*,bool);
protected:
	bool PreProcessLogicMessage( CPlayer*pPlayer ,stMsg* pmsg , unsigned int nSessionID );
	void PushReserverPlayers( CPlayer* ) ;
	CPlayer* GetReserverPlayer();
	void AddPlayer(CPlayer*);
	void ProcessWillRemovePlayer();
	void LogState();
	bool ProcessIsAlreadyLogin(unsigned int nUserID ,unsigned nSessionID ) ;
protected:
	// logic data ;
	MAP_PLAYERS m_vAllActivePlayers ;
	LIST_PLAYERS m_vAllReservePlayers ;
	LIST_PLAYERS m_vWillRemovePlayers ;
};