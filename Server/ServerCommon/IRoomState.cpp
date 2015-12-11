#include "IRoomState.h"
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