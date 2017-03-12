#pragma once
#include "NativeTypes.h"
#include <map>
#include <vector>
#include<memory>
#include "IGlobalModule.h"
class IGameRecorder
{
public:
	virtual void addPlayerOffset( uint32_t nUID , int32_t nOffset ) = 0 ;
	virtual uint32_t getSieralNum() = 0 ;
	virtual void doSave() = 0;
	virtual uint32_t getTimestamp() = 0;
};

class RoomRecorderEntery
	:public IGameRecorder
{
public:
	void setEnteryInfo( uint8_t nReplayUID, uint32_t nFinishTime);
	void addPlayerOffset(uint32_t nUID, int32_t nOffset)override;
	uint32_t getSieralNum()override;
	void doSave()override;
	uint32_t getTimestamp()override;
protected:
	std::map<uint32_t, int32_t> m_vPlayerOffset;   
	time_t m_tFinishTime;
	uint32_t m_tReplayUID;
};

class RoomRecorder
	:public IGameRecorder
{
public:
	void setRoomInfo(uint8_t nSieralNum , uint32_t nRoomID , uint8_t nRoomType );
	void addPlayerOffset(uint32_t nUID, int32_t nOffset)override;
	uint32_t getSieralNum()override;
	void doSave()override;
	uint32_t getTimestamp()override;
	void addRoomRecorderEntery( std::shared_ptr<RoomRecorderEntery> pEntry );
	uint32_t getCreatorUID();
protected:
	uint32_t m_nRoomID;
	uint32_t m_nRoomType;
	uint32_t m_nCreatorUID;
	uint32_t m_nSieralNum;
	std::map<uint32_t, int32_t> m_vPlayerOffset;
	time_t m_tFinishTime;
	std::vector<std::shared_ptr<RoomRecorderEntery>> m_vRoomRecorderEnteries;
};

class GameRecorderMgr
	:public IGlobalModule
{
public:
	GameRecorderMgr();
	void addRoomRecorder( std::shared_ptr<RoomRecorder> pRoomRecorder , bool isSaveToDB );
	std::shared_ptr<RoomRecorder> getRoomRecorder( uint32_t nSieralNum );
	bool onMsg(Json::Value& prealMsg, uint16_t nMsgType, eMsgPort eSenderPort, uint32_t nSessionID)override;
protected:
	std::map<uint32_t, std::shared_ptr<RoomRecorder>> m_vRoomRecorders;
};