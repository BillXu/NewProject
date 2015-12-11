#include "ISitableRoom.h"
#include "RoomConfig.h"
#include "ISitableRoomPlayer.h"
#include <cassert>
ISitableRoom::~ISitableRoom()
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx])
		{
			delete m_vSitdownPlayers[nIdx];
			m_vSitdownPlayers[nIdx] = nullptr ;
		}
	}
	delete [] m_vSitdownPlayers;

	for ( ISitableRoomPlayer* pPlayer : m_vReserveSitDownObject )
	{
		delete pPlayer ;
		pPlayer = nullptr ;
	}
	m_vReserveSitDownObject.clear() ;
}

bool ISitableRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID ) 
{
	IRoom::init(pConfig,nRoomID) ;
	stSitableRoomConfig* pC = (stSitableRoomConfig*)pConfig;
	m_nSeatCnt = pC->nMaxSeat ;
	m_vSitdownPlayers = new ISitableRoomPlayer*[m_nSeatCnt] ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		m_vSitdownPlayers[nIdx] = nullptr ;
	}
	return true ;
}

bool ISitableRoom::playerSitDown(ISitableRoomPlayer* pPlayer , uint8_t nIdx )
{
	if ( isSeatIdxEmpty(nIdx) )
	{
		m_vSitdownPlayers[nIdx] = pPlayer ;
		pPlayer->doSitdown(nIdx) ;
		pPlayer->setIdx(nIdx);
		return true ;
	}
	return false ;
}

void ISitableRoom::playerStandUp( ISitableRoomPlayer* pPlayer )
{
	assert(isSeatIdxEmpty(pPlayer->getIdx()) == false && "player not sit down" );
	pPlayer->willStandUp();
	m_vSitdownPlayers[pPlayer->getIdx()] = nullptr ;
	m_vReserveSitDownObject.push_back(pPlayer) ;
}

uint8_t ISitableRoom::getEmptySeatCount()
{
	uint8_t nIdx = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] == nullptr )
		{
			++nIdx ;
		}
	}
	return nIdx ;
}

ISitableRoomPlayer* ISitableRoom::getPlayerByIdx(uint8_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return nullptr ;
	}
	return m_vSitdownPlayers[nIdx] ;
}

bool ISitableRoom::isSeatIdxEmpty( uint8_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return false ;
	}
	return m_vSitdownPlayers[nIdx] == nullptr ;
}

uint8_t ISitableRoom::getSitDownPlayerCount()
{
	return getSeatCount() - getEmptySeatCount() ;
}

uint8_t ISitableRoom::getSeatCount()
{
	return m_nSeatCnt ;
}

ISitableRoomPlayer* ISitableRoom::getReuseSitableRoomPlayerObject()
{
	LIST_SITDOWN_PLAYERS::iterator iter = m_vReserveSitDownObject.begin() ;
	if ( iter != m_vReserveSitDownObject.end() )
	{
		ISitableRoomPlayer* p = *iter ;
		m_vReserveSitDownObject.erase(iter) ;
		return p ;
	}
	return doCreateSitableRoomPlayer();
}