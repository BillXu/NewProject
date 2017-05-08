#include "NiuNiuRoomDistribute4CardState.h"
#include "NiuNiuRoomTryBankerState.h"
#include "ISitableRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "log4z.h"
#include "NiuNiuRoom.h"
void CNiuNiuRoomDistribute4CardState::enterState(IRoom* pRoom)
{
	m_pRoom = (ISitableRoom*)pRoom ;
	((CNiuNiuRoom*)pRoom)->onDistributeCard(4);
	setStateDuringTime( m_pRoom->getPlayerCntWithState(eRoomPeer_CanAct) * TIME_NIUNIU_DISTRIBUTE_4_CARD_PER_PLAYER );
	LOGFMTD("room id = %d , distribute 4 card,m_pRoom->getRoomID()");
}

void CNiuNiuRoomDistribute4CardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(eRoomState_NN_GrabBanker) ;
}