#include "MJGameReplayManager.h"
#include "ISeverApp.h"
void MJReplayGame::sendToClient(IServerApp* pApp, uint32_t nSessionID)
{
	// base data 
	Json::Value jsInfo;
	jsInfo["replayID"] = m_nReplayID;
	jsInfo["ret"] = 0;
	jsInfo["roomInfo"] = m_jsGameInfo;
	pApp->sendMsg(nSessionID, jsInfo, MSG_REQ_GAME_REPLAY );

	// send frame data 
	Json::Value jsFrameData;
	jsFrameData["replayID"] = m_nReplayID;
	uint32_t nSended = 0;
	for ( auto& ref : m_vAllFrames )
	{
		jsFrameData["isEnd"] = m_vAllFrames.size() == ( nSended + 1) ;
		ref->toJson(jsFrameData["data"]);
		pApp->sendMsg(nSessionID, jsFrameData, MSG_REPLAY_FRAME );
		++nSended;
	}
}

// game mgr 
bool MJGameReplayManager::onMsg( Json::Value & prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID )
{
	if ( nMsgType == MSG_REQ_GAME_REPLAY )
	{
		auto nReplayID = prealMsg["replayID"].asUInt();
		auto iter = m_vAllGames.find(nReplayID);
		if (iter == m_vAllGames.end())
		{
			Json::Value jsInfo;
			jsInfo["replayID"] = nReplayID;
			jsInfo["ret"] = 1;
			getSvrApp()->sendMsg(nSessionID, jsInfo, MSG_REQ_GAME_REPLAY);
		}
		else
		{
			iter->second->sendToClient(getSvrApp(), nSessionID);
		}
		return true;
	}
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
