#pragma once
#include "IGameReplay.h"
#include "IGlobalModule.h"
class MJReplayGame
	:public IReplayGame
{
public:
	void setReplayRoomInfo(Json::Value& jsInfo, uint32_t nReplayID ) override;
	uint32_t getReplayID()override;
	void addFrame(std::shared_ptr<IReplayFrame> ptrFrame) override;
	void restore(Json::Value& jsReplay) override;
	void toJson(Json::Value& jsReplay) override;
	std::shared_ptr<IReplayFrame> createFrame(uint16_t nFrameType, uint32_t nTimestamp) override ;
protected:
	Json::Value m_jsGameInfo;
	uint32_t m_nReplayID;
};

class MJGameReplayManager
	:public IGlobalModule
{
public:
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override;
	void onConnectedSvr()override;
	void addGameReplay( std::shared_ptr<IReplayGame> ptrGameReplay );
protected:
	std::map<uint32_t, std::shared_ptr<IReplayGame>> m_vAllGames;
};




