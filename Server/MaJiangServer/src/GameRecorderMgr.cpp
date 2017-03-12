#include "GameRecorderMgr.h"
#include "log4z.h"
void RoomRecorderEntery::setEnteryInfo(uint8_t nReplayUID, uint32_t nFinishTime)
{
	m_tReplayUID = nReplayUID;
	m_tFinishTime = nFinishTime;
	m_vPlayerOffset.clear();
}

void RoomRecorderEntery::addPlayerOffset(uint32_t nUID, int32_t nOffset)
{
	m_vPlayerOffset[nUID] = nOffset;
}

uint32_t RoomRecorderEntery::getSieralNum()
{
	return m_tReplayUID;
}

void RoomRecorderEntery::doSave()
{
	// do save this entry ;
}

uint32_t RoomRecorderEntery::getTimestamp()
{
	return m_tFinishTime;
}

// room recorder 
void RoomRecorder::setRoomInfo(uint8_t nSieralNum, uint32_t nRoomID, uint8_t nRoomType )
{
	m_nSieralNum = nSieralNum;
	m_nRoomID = nRoomID;
	m_nRoomType = nRoomType;
	m_nCreatorUID = 0;
	m_tFinishTime = 0;
}

void RoomRecorder::addPlayerOffset(uint32_t nUID, int32_t nOffset)
{
	m_vPlayerOffset[nUID] = nOffset;
}

uint32_t RoomRecorder::getSieralNum()
{
	return m_nSieralNum;
}

void RoomRecorder::doSave()
{
	// save game recorder ;
}

uint32_t RoomRecorder::getTimestamp()
{
	return m_nSieralNum;
}

void RoomRecorder::addRoomRecorderEntery(std::shared_ptr<RoomRecorderEntery> pEntry)
{
	m_vRoomRecorderEnteries.push_back(pEntry);
}

uint32_t RoomRecorder::getCreatorUID()
{
	return m_nCreatorUID;
}

// reoom recorder mgr ;

GameRecorderMgr::GameRecorderMgr()
{
	m_vRoomRecorders.clear();
}

void GameRecorderMgr::addRoomRecorder(std::shared_ptr<RoomRecorder> pRoomRecorder, bool isSaveToDB)
{
	auto p = getRoomRecorder(pRoomRecorder->getSieralNum());
	if (p)
	{
		LOGFMTE("you already add this room recorder , why add two times sieral = %u , creator uid = %u ",p->getSieralNum(),p->getCreatorUID() );
		return;
	}
	m_vRoomRecorders[pRoomRecorder->getSieralNum()] = pRoomRecorder;
}

std::shared_ptr<RoomRecorder> GameRecorderMgr::getRoomRecorder(uint32_t nSieralNum)
{
	auto iter = m_vRoomRecorders.find(nSieralNum);
	if (iter != m_vRoomRecorders.end())
	{
		return iter->second;
	}
	return nullptr;
}

bool GameRecorderMgr::onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)
{
	return false;
}