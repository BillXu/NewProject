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