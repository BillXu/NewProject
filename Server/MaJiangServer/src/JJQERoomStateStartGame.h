#pragma once
#include "MJRoomStateStartGame.h"
#include "JJQERoom.h"
class JJQERoomStateStartGame
	:public MJRoomStateStartGame
{
public:
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		auto pRoom = (JJQERoom*)getRoom();
		if ( false == pRoom->isEnableChaoZhuang())  // if chao zhuang mode , willstartGame already invokde in chao zhuang state 
		{
			getRoom()->willStartGame();
		}
		getRoom()->startGame();
		setStateDuringTime(eTime_ExeGameStart);
	}

	void onStateTimeUp()
	{
		getRoom()->goToState(eRoomState_WaitPlayerQiPai);
		
		/*auto pRoom = (JJQERoom*)getRoom();
		if ( pRoom->isAnyPlayerFlyUp() )
		{
			getRoom()->goToState(eRoomState_WaitPlayerFlyUp);
		}
		else if ( pRoom->isAnyPlayerAutoBuHua() )
		{
			getRoom()->goToState(eRoomState_NJ_Auto_Buhua);
		}
		else
		{
			Json::Value jsValue;
			jsValue["idx"] = getRoom()->getBankerIdx();
			getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
		}*/

	}
};
