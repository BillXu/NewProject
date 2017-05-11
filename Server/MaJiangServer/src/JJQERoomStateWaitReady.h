#pragma once
#include "MJRoomStateWaitReady.h"
#include "JJQERoom.h"
class JJQERoomStateWaitReady
	:public CMJRoomStateWaitReady
{
public:
	void onAllPlayerReady()override 
	{
		auto pRoom = (JJQERoom*)getRoom();
		if ( pRoom && pRoom->isEnableChaoZhuang() )
		{
			getRoom()->goToState(eRoomState_WaitChaoZhuang);
		}
		else
		{
			CMJRoomStateWaitReady::onAllPlayerReady();
		}
	}
};