#pragma once
#include "IRoom.h"
struct stSitableRoomConfig ;
class ISitableRoomPlayer ;
class ISitableRoom
	:public IRoom
{
public:
	typedef std::list<ISitableRoomPlayer*> LIST_SITDOWN_PLAYERS ;
public:
	~ISitableRoom();
	bool init(stBaseRoomConfig* pConfig, uint32_t nRoomID ) override;
	bool playerSitDown(ISitableRoomPlayer* pPlayer , uint8_t nIdx );
	void playerStandUp( ISitableRoomPlayer* pPlayer );
	uint8_t getEmptySeatCount();
	ISitableRoomPlayer* getPlayerByIdx(uint8_t nIdx );
	bool isSeatIdxEmpty( uint8_t nIdx );
	uint8_t getSitDownPlayerCount();
	uint8_t getSeatCount();
	ISitableRoomPlayer* getReuseSitableRoomPlayerObject();
	virtual ISitableRoomPlayer* doCreateSitableRoomPlayer() = 0 ;
	uint8_t getPlayerCntWithState( uint32_t nState );
protected:
	uint8_t m_nSeatCnt ;
	ISitableRoomPlayer** m_vSitdownPlayers ;
private:
	LIST_SITDOWN_PLAYERS m_vReserveSitDownObject ;
};