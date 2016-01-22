#include "IRoomState.h"
#include "IRoom.h"
#include <time.h>
#include "ServerCommon.h"
void IRoomState::update(float fDeta)
{ 
	if ( m_fStateDuring > 0 )
	{ 
		m_fStateDuring -= fDeta ; 
		if ( m_fStateDuring <= 0 )
		{ 
			onStateDuringTimeUp();
		} 
	}
}


void IRoomStateDead::enterState(IRoom* pRoom)
{
	m_pRoom = pRoom; 

	// send inform ;
	if ( pRoom->getOwnerUID() == MATCH_MGR_UID )  // match room 
	{
		pRoom->onMatchFinish();
		m_MatchRoomDuringTime = pRoom->getDeadTime() - pRoom->getCreateTime();
		m_fMatchRestarTime = TIME_MATCH_PAUSE ; // 30 min later start new match ;
	}
	else
	{
		pRoom->sendExpireInform();
	}
} 

void IRoomStateDead::update(float fDeta)
{ 
	if ( m_pRoom->isDeleteRoom() == false  && m_pRoom->getOwnerUID() != MATCH_MGR_UID)
	{
		time_t deadTime = m_pRoom->getDeadTime() ;
		if ( time(nullptr) - deadTime > 8640*3 ) // 3 days ;
		{
			m_pRoom->deleteRoom();
		}
	}

	if ( m_pRoom->isRoomAlive() )
	{
		m_pRoom->goToState(eRoomState_WaitJoin);
	}
	else
	{
		if ( m_pRoom->getOwnerUID() == MATCH_MGR_UID )
		{
			m_fMatchRestarTime -= fDeta;
			if ( m_fMatchRestarTime <= 0 )
			{
				m_pRoom->setCreateTime(time(nullptr));
				m_pRoom->setDeadTime(m_pRoom->getCreateTime() + m_MatchRoomDuringTime) ;
				m_pRoom->onMatchRestart();
			}
		}
	}
}