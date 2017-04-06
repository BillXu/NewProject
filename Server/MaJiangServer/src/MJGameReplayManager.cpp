#include "MJGameReplayManager.h"
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
	m_vAllGames[ptrGameReplay->getReplayID()] = ptrGameReplay;
}

uint32_t MJGameReplayManager::generateReplayID()
{
	return ++m_nMaxReplayUID;
}
