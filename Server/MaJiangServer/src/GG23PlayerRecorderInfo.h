#pragma once
#pragma once
#include "IGameRecorder.h"
#include "log4z.h"
class GG23PlayerRecorderInfo
	:public IPlayerReocrderInfo
{
public:
	void init(uint32_t nUserUID, int32_t nOffset)
	{
		setInfo(nUserUID, nOffset);
		m_nHuCnts = 0;
	}

	void setInfo(uint32_t nUserUID, int32_t nOffset)override
	{
		m_nUserUID = nUserUID;
		m_nOffset = nOffset;
	}

	void setHuCnts(uint16_t nHuCnts)
	{
		m_nHuCnts = nHuCnts;
	}

	void getUserDetailForSave(Json::Value& jsUserDetail)
	{
		jsUserDetail["huCnt"] = m_nHuCnts;
	}

	void restoreUserDetail(Json::Value& jsUserDetail)
	{
		if (jsUserDetail["huCnt"].isNull() == false)
		{
			m_nHuCnts = jsUserDetail["huCnt"].asUInt();
		}
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
		jsUserInfo["huCnt"] = m_nHuCnts;
	}

protected:
	uint32_t m_nUserUID;
	int32_t m_nOffset;
	uint16_t m_nHuCnts;
};

// single recorder 
class GG23SingleRoundRecorder
	:public ISingleRoundRecorder
{
public:
	std::shared_ptr<IPlayerReocrderInfo> createPlayerRecorderInfo()override
	{
		return std::make_shared<GG23PlayerRecorderInfo>();
	}
};

// room recorder 
class GG23RoomRecorder
	:public IGameRoomRecorder
{
	void init(uint32_t nSieralNum, uint32_t nCirleCnt, uint32_t nRoomID, uint32_t nRoomType, uint32_t nCreaterUID)override
	{
		IGameRoomRecorder::init(nSieralNum, nCirleCnt, nRoomID, nRoomType, nCreaterUID);

	}

	void setRoomOpts(Json::Value& jsRoomOpts)override
	{
		m_jsRoomOpts["qiHuNeed"] = jsRoomOpts["qiHuNeed"];
		m_jsRoomOpts["liangPai"] = jsRoomOpts["liangPai"];
	}

	std::shared_ptr<ISingleRoundRecorder> createSingleRoundRecorder()override
	{
		return std::make_shared<GG23SingleRoundRecorder>();
	}

	void buildZhanJiMsg(Json::Value& jsMsg)override
	{
		IGameRoomRecorder::buildZhanJiMsg(jsMsg);

		//Json::Value jsRoomOpts;

		//jsRoomOpts["qiHuNeed"] = m_nQiHuNeed;
		//jsRoomOpts["fengDing"] = m_nTopLimit;

		//jsRoomOpts["chaoZhuangLevel"] = m_nChaoZhuangLevel ;
		//jsRoomOpts["qingErHu"] = m_nQingErHuCnt;

		//jsRoomOpts["is13Hu"] = m_isHave13Hu ? 1 : 0;
		//jsRoomOpts["haveSun"] = m_isHaveSun ? 1 : 0;

		//jsRoomOpts["haveMoon"] = m_isHaveMoon ? 1 : 0;

		jsMsg["roomOpts"] = m_jsRoomOpts;
	}

protected:
	void getRoomInfoOptsForSave(Json::Value& jsRoomOpts) override
	{
		//jsRoomOpts["qiHuNeed"] = m_nQiHuNeed;
		//jsRoomOpts["fengDing"] = m_nTopLimit;

		//jsRoomOpts["chaoZhuangLevel"] = m_nChaoZhuangLevel;
		//jsRoomOpts["qingErHu"] = m_nQingErHuCnt;

		//jsRoomOpts["is13Hu"] = m_isHave13Hu ;
		//jsRoomOpts["haveSun"] = m_isHaveSun  ;

		//jsRoomOpts["haveMoon"] = m_isHaveMoon  ;
		 jsRoomOpts = m_jsRoomOpts;
	};

	void restoreRoomInfoOpts(Json::Value& jsRoomOpts) override
	{
		if (jsRoomOpts.isNull())
		{
			LOGFMTE("room opts is nullptr ");
			return;
		}

		//m_isHave13Hu = jsRoomOpts["is13Hu"].asBool();
		//m_isHaveSun = jsRoomOpts["haveSun"].asBool();
		//m_isHaveMoon = jsRoomOpts["haveMoon"].asBool();

		//m_nChaoZhuangLevel = jsRoomOpts["qingErHu"].asUInt();

		//m_nChaoZhuangLevel = jsRoomOpts["chaoZhuangLevel"].asUInt();
		//m_nTopLimit = jsRoomOpts["fengDing"].asUInt();

		//m_nQiHuNeed = jsRoomOpts["qiHuNeed"].asUInt();
		 m_jsRoomOpts = jsRoomOpts;
	};
protected:
	//uint16_t m_nQiHuNeed;
	//uint16_t m_nTopLimit;
	//uint8_t m_nChaoZhuangLevel;  // 0 means , not chao zhuang ;
	//uint16_t m_nQingErHuCnt;
	//bool m_isHave13Hu;
	//bool m_isHaveSun;
	//bool m_isHaveMoon;

	Json::Value m_jsRoomOpts;
};