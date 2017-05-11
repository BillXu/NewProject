#pragma once
#include "IMJRoomState.h"
#include "JJQERoom.h"
class JJQERoomStateAutoBuHua
	:public IMJRoomState
{
public:
	uint32_t getStateID()final { return eRoomState_NJ_Auto_Buhua; }

	void enterState(IMJRoom* pmjRoom, Json::Value& jsTranData)override
	{
		IMJRoomState::enterState(pmjRoom, jsTranData);
		setStateDuringTime(eTime_DoPlayerMoPai);
		auto pRoom = (JJQERoom*)getRoom();
		pRoom->onDoAllPlayersAutoBuHua();
	}

	void onStateTimeUp()override
	{
		Json::Value jsValue;
		jsValue["idx"] = getRoom()->getBankerIdx();
		getRoom()->goToState(eRoomState_WaitPlayerAct, &jsValue);
	}
};