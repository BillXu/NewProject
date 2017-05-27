#pragma once
#include "IGameRecorder.h"
#include "log4z.h"
class JJQEPlayerRecorderInfo
	:public IPlayerReocrderInfo
{
public:
	void init(uint32_t nUserUID, int32_t nOffset)
	{
		setInfo(nUserUID, nOffset);
	}

	void setInfo(uint32_t nUserUID, int32_t nOffset)override
	{
		m_nUserUID = nUserUID;
		m_nOffset = nOffset;
	}

	void getUserDetailForSave(Json::Value& jsUserDetail)
	{

	}

	void restoreUserDetail(Json::Value& jsUserDetail)
	{

	}

	uint32_t getUserUID()override
	{
		return m_nUserUID;
	}

	int32_t getOffset()override
	{
		return m_nOffset;
	}

	void buildInfoForZhanJi(Json::Value& jsUserInfo)override
	{
		jsUserInfo["uid"] = m_nUserUID;
		jsUserInfo["offset"] = m_nOffset;
	}

protected:
	uint32_t m_nUserUID;
	int32_t m_nOffset;
};

// single recorder 
class JJQESingleRoundRecorder
	:public ISingleRoundRecorder
{
public:
	std::shared_ptr<IPlayerReocrderInfo> createPlayerRecorderInfo()override
	{
		return std::make_shared<JJQEPlayerRecorderInfo>();
	}
};

// room recorder 
class JJQERoomRecorder
	:public IGameRoomRecorder
{
	void init(uint32_t nSieralNum, uint32_t nCirleCnt, uint32_t nRoomID, uint32_t nRoomType, uint32_t nCreaterUID)override
	{
		IGameRoomRecorder::init(nSieralNum, nCirleCnt, nRoomID, nRoomType, nCreaterUID);

	}

	std::shared_ptr<ISingleRoundRecorder> createSingleRoundRecorder()override
	{
		return std::make_shared<JJQESingleRoundRecorder>();
	}

protected:
	void getRoomInfoOptsForSave(Json::Value& jsRoomOpts) override
	{

	};

	void restoreRoomInfoOpts(Json::Value& jsRoomOpts) override
	{
		if (jsRoomOpts.isNull())
		{
			LOGFMTE("room opts is nullptr ");
			return;
		}
	};
protected:

};