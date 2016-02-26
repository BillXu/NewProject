#include "ISitableRoomPlayer.h"
#define MAX_NEW_PLAYER_HALO 100
void ISitableRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	nUserUID = pPlayer->nUserUID ;
	nSessionID = pPlayer->nUserSessionID ;
	nNewPlayerHaloWeight = pPlayer->nNewPlayerHaloWeight ;
	nPlayTimes = nWinTimes = 0 ;
	nSingleWinMost = 0 ;
	m_nHaloState = 0 ;
}

void ISitableRoomPlayer::onGameEnd()
{
	m_nHaloState = 0 ;
}

bool ISitableRoomPlayer::isHaveHalo()
{
	if ( nNewPlayerHaloWeight <= 0 )
	{
		return false ;
	}

	if ( m_nHaloState == 2 )
	{
		return false ;
	}

	if ( 1 == m_nHaloState )
	{
		return true ;
	}

	uint32_t nRate = rand() % ( MAX_NEW_PLAYER_HALO + 1 );
	m_nHaloState = nRate <= nNewPlayerHaloWeight ? 1 : 2 ;
	return m_nHaloState == 1 ;
}
