#include "MJGameReplayManager.h"
// reaplay game 
void MJReplayGame::setReplayRoomInfo(Json::Value& jsInfo, uint32_t nReplayID )
{

}

uint32_t MJReplayGame::getReplayID()
{

}

void MJReplayGame::addFrame(std::shared_ptr<IReplayFrame> ptrFrame)
{

}

void MJReplayGame::restore(Json::Value& jsReplay)
{

}

void MJReplayGame::toJson(Json::Value& jsReplay)
{

}

std::shared_ptr<IReplayFrame> MJReplayGame::createFrame(uint16_t nFrameType, uint32_t nTimestamp)
{

}

// game mgr 
bool MJGameReplayManager::onMsg(Json::Value & prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
{
	return false;
}

void MJGameReplayManager::onConnectedSvr()
{
}

void MJGameReplayManager::addGameReplay(std::shared_ptr<IReplayGame> ptrGameReplay)
{
}
