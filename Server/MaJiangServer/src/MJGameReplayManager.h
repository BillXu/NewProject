#pragma once
#include "IGameReplay.h"
#include "IGlobalModule.h"
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



