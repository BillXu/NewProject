#include "NiuNiuRoomWaitJoinState.h"
#include "ISitableRoom.h"
#include "NiuNiuRoomDistribute4CardState.h"
void CNiuNiuRoomWaitJoinState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;
}

void CNiuNiuRoomWaitJoinState::update( float fDeta )
{
	if ( m_pRoom && m_pRoom->getSitDownPlayerCount() >= 2 )
	{
		m_pRoom->goToState(CNiuNiuRoomDistribute4CardState::eStateID);
	}
}