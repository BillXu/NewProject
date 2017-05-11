#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class JJQERoom
	:public IMJRoom
{
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	uint8_t getRoomType()override { return eRoom_MJ_QingEr; }
	bool isEnableChaoZhuang();
	void onLastChaoZhuangPlayerViewCard( uint8_t nCard , bool isHua );
	bool isAnyPlayerFlyUp();
	bool isAnyPlayerAutoBuHua();
	bool informPlayerFlyUp( uint8_t nPlayerIdx );
	bool onPlayerDoFlyUp( uint8_t nIdx , std::vector<uint8_t>& vFlyUpCard );
	void onDoAllPlayersAutoBuHua();
};