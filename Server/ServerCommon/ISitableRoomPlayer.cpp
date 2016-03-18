#include "ISitableRoomPlayer.h"
#include "LogManager.h"
#include "ServerCommon.h"
void ISitableRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	nUserUID = pPlayer->nUserUID ;
	nSessionID = pPlayer->nUserSessionID ;
	nNewPlayerHaloWeight = pPlayer->nNewPlayerHaloWeight ;
	nPlayTimes = nWinTimes = 0 ;
	nSingleWinMost = 0 ;
	m_nHaloState = 0 ;
	nCoin = 0 ;
	m_nIdx = 0 ;
	m_nState = 0 ;
	nTempHaloWeight = 0 ;
	m_isDelayStandUp = false ;
}

void ISitableRoomPlayer::onGameEnd()
{
	m_nHaloState = 0 ;
	if ( nNewPlayerHaloWeight > 0 )
	{
		--nNewPlayerHaloWeight;
	}
}

bool ISitableRoomPlayer::isHaveHalo()
{
	if ( getTotalHaloWeight() <= 0 )
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
	m_nHaloState = nRate <= getTotalHaloWeight() ? 1 : 2 ;
	if ( m_nHaloState == 1 )
	{
		CLogMgr::SharedLogMgr()->PrintLog("uid = %u invoke halo tempHalo = %u",getUserUID(),nTempHaloWeight);
	}
	nTempHaloWeight = 0 ;
	return m_nHaloState == 1 ;
}
