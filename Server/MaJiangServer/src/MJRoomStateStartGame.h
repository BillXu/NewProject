#pragma once 
#include "IMJRoomState.h"
#include "IMJRoom.h"
class MJRoomStateStartGame
	:public IMJRoomState
{
public:
	uint32_t getStateID(){ return eRoomState_StartGame; }
	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		getRoom()->willStartGame();
		getRoom()->startGame();
		setStateDuringTime(eTime_ExeGameStart);
	}

	void onStateTimeUp()
	{
		Json::Value jsValue;
		jsValue["idx"] = getRoom()->getBankerIdx();
		getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
	}
	uint8_t getCurIdx()override{ return 0; }
};

