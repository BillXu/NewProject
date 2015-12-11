#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
#include "CardPoker.h"
#include <list>
#include <map>
class IRoomState ;
class IRoomPlayer ;
struct stMsg ;
struct stBaseRoomConfig ;
class IRoom
{
public:
	typedef std::list<IRoomPlayer*> LIST_ROOM_PLAYER ;
	typedef std::map<uint32_t,IRoomPlayer*> MAP_UID_ROOM_PLAYER ;
	typedef std::map<uint16_t,IRoomState*>	MAP_ID_ROOM_STATE;
	typedef MAP_UID_ROOM_PLAYER::iterator PLAYER_ITER ;
public:
	IRoom();
	virtual ~IRoom();
	virtual bool init(stBaseRoomConfig* pConfig, uint32_t nRoomID );
	uint32_t getRoomID();
	virtual void update(float fDelta);

	bool addRoomPlayer(IRoomPlayer* pPlayer );
	void removePlayer(IRoomPlayer* pPlayer );
	IRoomPlayer* getPlayerByUserUID(uint32_t nUserUID );
	IRoomPlayer* getPlayerBySessionID(uint32_t nSessionID );
	bool isPlayerInRoom(IRoomPlayer* pPlayer );
	bool isPlayerInRoomWithSessionID(uint32_t nSessioID );
	bool isPlayerInRoomWithUserUID(uint32_t nUserUID );
	uint16_t getPlayerCount();
	IRoomPlayer* getReusePlayerObject();
	virtual IRoomPlayer* doCreateRoomPlayerObject() = 0 ;
	PLAYER_ITER beginIterForPlayers();
	PLAYER_ITER endIterForPlayers();

	void sendRoomMsg( stMsg* pmsg , uint16_t nLen );
	virtual void sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID ) = 0 ;
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	virtual void sendRoomInfoToPlayer(uint32_t nSessionID) = 0 ;
	virtual void onGameWillBegin(){}
	virtual void onGameDidEnd(){}

	virtual void onTimeSave();
	void goToState(IRoomState* pTargetState );
	void goToState( uint16_t nStateID );
	void setInitState(IRoomState* pDefaultState );
	IRoomState* getCurRoomState();
	IRoomState* getRoomStateByID(uint16_t nStateID );
	CPoker* getPoker(){ return &m_tPoker ; }
protected:
	bool addRoomState(IRoomState* pRoomState );
private:
	uint32_t m_nRoomID ;
	LIST_ROOM_PLAYER m_vReseverPlayerObjects;
	MAP_UID_ROOM_PLAYER m_vInRoomPlayers ;
	IRoomState* m_pCurRoomState ;
	MAP_ID_ROOM_STATE m_vRoomStates ;
	CPoker m_tPoker ;
};