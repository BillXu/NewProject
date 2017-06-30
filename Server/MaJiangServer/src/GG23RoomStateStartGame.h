#pragma once
#include "MJRoomStateStartGame.h"
#include "GG23Room.h"
class GG23RoomStateStartGame
	:public MJRoomStateStartGame
{
public:
	void onStateTimeUp()
	{
		auto pRoom = (GG23Room*)getRoom();
		if ( pRoom->isAnyPlayerFlyUp() )
		{
			getRoom()->goToState(eRoomState_WaitPlayerFlyUp);
			return;
		}

		MJRoomStateStartGame::onStateTimeUp();
	}
};

