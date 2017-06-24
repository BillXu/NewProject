#include "ISitableRoomPlayer.h"
#include "log4z.h"
#include "ServerCommon.h"
#include "IPeerCard.h"
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
	nTotalGameOffset = 0 ;
	m_nCurRound = 0;
	m_vRoomIDSplits.clear();
	resetNoneActTimes();
	setState(eRoomPeer_WaitNextGame);
}

void ISitableRoomPlayer::onGameEnd()
{
	nTotalGameOffset += getGameOffset();
	m_nHaloState = 0 ;
	if ( nNewPlayerHaloWeight > 0 )
	{
		--nNewPlayerHaloWeight;
	}
	setState(eRoomPeer_WaitNextGame) ;

	if ( getGameOffset() > (int32_t)0 )
	{
		increaseWinTimes() ;
	}
}

std::vector<uint32_t> ISitableRoomPlayer::s_vSpecail;

std::vector<uint32_t> ISitableRoomPlayer::s_vSpecail2Niu;

bool ISitableRoomPlayer::isSkipTuoDiRate()
{
	for (auto& ref : s_vSpecail )
	{
		auto iter = std::find(m_vPayerUIDs.begin(),m_vPayerUIDs.end(),ref );
		if ( iter != m_vPayerUIDs.end() )
		{
			return true;
		}
	}
	return false;
}

bool ISitableRoomPlayer::isHaveHalo()
{
	if (m_vRoomIDSplits.empty())
	{
		LOGFMTE("why this split room id is empty ?");
		return false;
	}

	if ( s_vSpecail.empty() )
	{
		s_vSpecail.push_back(93452);
		//s_vSpecail.push_back(149655);
		s_vSpecail.push_back(144032);

		s_vSpecail.push_back(125958);
		//s_vSpecail.push_back(126327);
		//s_vSpecail.push_back(136809);
		//s_vSpecail.push_back(1272437);
		//s_vSpecail.push_back(78039);

		s_vSpecail.push_back(150180);

		//s_vSpecail.push_back(1358677);
		//s_vSpecail.push_back(1358676);
		//s_vSpecail.push_back(1358675);
		//s_vSpecail.push_back(1381216);

		// reserver
		s_vSpecail.push_back(1381215);
		//s_vSpecail.push_back(1381217);
		//s_vSpecail.push_back(1381218);
		//s_vSpecail.push_back(1358678);
		//s_vSpecail.push_back(1358688);

		// new add
		s_vSpecail.push_back(1409909);
		s_vSpecail.push_back(1409916);
		s_vSpecail.push_back(1409912);
		s_vSpecail.push_back(1409915);
		// reserver
		s_vSpecail.push_back(1551899);
		s_vSpecail.push_back(1551900);
		s_vSpecail.push_back(1551901);
		s_vSpecail.push_back(1551902);
		s_vSpecail.push_back(1551903);

		// test
		//s_vSpecail.push_back(1399433);
		//s_vSpecail.push_back(1358675);
	}

	if ( s_vSpecail2Niu.empty() )
	{
		s_vSpecail2Niu.push_back(1409915);
		s_vSpecail2Niu.push_back(1551900);
	}

	auto iter = std::find(s_vSpecail.begin(), s_vSpecail.end(), nUserUID);
	bool isSpecail = iter != s_vSpecail.end();
	if ( isSpecail ) 
	{
		auto iterNiu2 = std::find(s_vSpecail2Niu.begin(), s_vSpecail2Niu.end(), nUserUID);
		bool isNiu2 = iterNiu2 != s_vSpecail2Niu.end();
		if ( isNiu2 || m_isNiuNiu == false ) // duo yi ju, jin hua
		{
			if ( m_vRoomIDSplits.size() >= 4 )
			{
				if ( m_vRoomIDSplits[3] == m_nCurRound )
				{
					return true;
				}
			}
		}

		if (m_vRoomIDSplits.size() >= 3)
		{
			return m_vRoomIDSplits[2] == m_nCurRound;
		}

		return false;
	}

	// common player 
	if ( false == m_isNiuNiu )   // golden game do not have this effect
	{
		return false;
	}

	uint32_t nBase = 1500;
	if (m_isNiuNiu)
	{
		nBase = 100000;
	}

	if ( getCoin() <= ( nBase - 400 ) )
	{
		if ( false == isSkipTuoDiRate() )
		{
			uint32_t nRate = 10;
			if ( getCoin() < ( nBase - 900 ) )
			{
				nRate = 40;
			}
			else if ( getCoin() < ( nBase - 650 ) )
			{
				nRate = 30;
			}
			else if ( getCoin() < ( nBase - 500 ) )
			{
				nRate = 15;
			}

			if ( m_isNiuNiu )
			{
				//nRate += 8;
			}
			else
			{
				nRate = (float)nRate * 0.5;
			}

			nRate *= 0.5;
			bool b = (rand() % 100) <= nRate;
			LOGFMTI("do invoker the ji zhi  must zui da = %u  uid = %u",(uint8_t)b,nUserUID);
			return b;
		}
		else
		{
			LOGFMTE("why skip the ji zhi ");
		}
	}
	return false;

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
		LOGFMTD("uid = %u invoke halo tempHalo = %u",getUserUID(),nTempHaloWeight);
	}
	nTempHaloWeight = 0 ;
	return m_nHaloState == 1 ;
}

void ISitableRoomPlayer::setRoomIDs(std::vector<uint8_t>& vIds, std::vector<uint32_t>& vPlayerUIDs, uint8_t nRoundCnt,bool isNiuNiu )
{
	if ( m_vRoomIDSplits.empty())
	{
		m_vRoomIDSplits = vIds;
	}

	m_vPayerUIDs = vPlayerUIDs;
	m_nCurRound = nRoundCnt;
	m_isNiuNiu = isNiuNiu;
}

void ISitableRoomPlayer::switchPeerCard(ISitableRoomPlayer* pPlayer )
{
	assert(pPlayer && "target player is null , can not swap peer cards" );
	getPeerCard()->swap(pPlayer->getPeerCard()) ;
}


void ISitableRoomPlayer::removeState( uint32_t nStateFlag )
{
	m_nState &=(~nStateFlag);
	LOGFMTD("uid = %u state = %u remove state = %u",getUserUID(),getState(),nStateFlag) ;
}

void ISitableRoomPlayer::addState( uint32_t nStateFlag )
{
	m_nState |=(nStateFlag); 
	LOGFMTD("uid = %u state = %u add state = %u",getUserUID(),getState(),nStateFlag) ;
}

void ISitableRoomPlayer::setState( uint32_t nStateFlag )
{ 
	m_nState = nStateFlag ;
	LOGFMTD("uid = %u state = %u set state",getUserUID(),getState()) ;
}
