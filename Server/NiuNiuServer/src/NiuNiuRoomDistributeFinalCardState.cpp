#include "NiuNiuRoomDistributeFinalCardState.h"
#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuRoomPlayerCaculateCardState.h"
#include "NiuNiuMessageDefine.h"
#include "AutoBuffer.h"
#include "log4z.h"
void CNiuNiuRoomDistributeFinalCardState::enterState(IRoom* pRoom)
{
	LOGFMTD("room id = %d start final card ",pRoom->getRoomID()) ;

	m_pRoom = (CNiuNiuRoom*)pRoom ;

	setStateDuringTime(1 * TIME_NIUNIU_DISTRIBUTE_FINAL_CARD_PER_PLAYER );
	m_pRoom->onDistributeCard(5);
	LOGFMTD("room id = %d , distribute 5 card", m_pRoom->getRoomID());
}

void CNiuNiuRoomDistributeFinalCardState::onStateDuringTimeUp()
{
	m_pRoom->goToState(CNiuNiuRoomStatePlayerCaculateCardState::eStateID) ;
}