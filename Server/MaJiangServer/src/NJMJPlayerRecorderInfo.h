#pragma once
#include "IGameRecorder.h"
class NJMJPlayerRecorderInfo
	:public IPlayerReocrderInfo
{
public:
	void init( uint32_t nUserUID, int32_t nOffset, int32_t nWaiBaoOffset )
	{
		setInfo(nUserUID,nOffset);
		m_nWaiBaoOffset = nWaiBaoOffset;
	}

	void setInfo(uint32_t nUserUID, int32_t nOffset)override
	{
		m_nUserUID = nUserUID;
		m_nOffset = nOffset;
	}

	uint32_t getUserUID()override
	{
		return m_nUserUID;
	}

	int32_t getOffset()override 
	{
		return m_nOffset;
	}

	int32_t getWaiBaoOffset()
	{
		return m_nWaiBaoOffset;
	}

	void getUserDetailForSave(Json::Value& jsUserDetail)override
	{
		jsUserDetail["waibao"] = m_nWaiBaoOffset;
	}

	void restoreUserDetail(Json::Value& jsUserDetail)override
	{
		m_nWaiBaoOffset = jsUserDetail["waibao"].asInt();
	}

	void buildInfoForZhanJi( Json::Value& jsUserInfo )override
	{
		jsUserInfo["uid"] = m_nUserUID;
		if ( 0 != m_nOffset)
		{
			jsUserInfo["offset"] = m_nOffset;
		}
		
		if (m_nWaiBaoOffset != 0)
		{
			jsUserInfo["waiBaoOffset"] = m_nWaiBaoOffset;
		}
	}

protected:
	uint32_t m_nUserUID;
	int32_t m_nOffset;
	int32_t m_nWaiBaoOffset;
};

// single recorder 
class NJMJSingleRoundRecorder
	:public ISingleRoundRecorder
{
public:
	std::shared_ptr<IPlayerReocrderInfo> createPlayerRecorderInfo()override
	{
		return std::make_shared<NJMJPlayerRecorderInfo>();
	}
};

// room recorder 
class NJMJRoomRecorder
	:public IGameRoomRecorder
{
public:
	void init(uint32_t nSieralNum, uint32_t nCirleCnt, uint32_t nRoomID, uint32_t nRoomType, uint32_t nCreaterUID)override
	{
		IGameRoomRecorder::init(nSieralNum, nCirleCnt,nRoomID,nRoomType,nCreaterUID);
		m_isJinYuanZi = false;
		m_nJinYuanZiCoin = 0;
		
		m_isKuaiChong = false;
		m_nKuaiChongPool = 0;

		m_isBiXiaHu = false;
		m_isHuaZa = false;
		m_isCircleType = true;
		m_isJieZhuangBi = false;
		m_isEnableShuangGang = false;
		m_isEnableYiZhanDaoDi = false;
	}

	void setJingYuanZi(bool isJingYuanZi, uint32_t nYuanZiCoin, bool isWaiBao , bool isShuangGAng , bool isYiDuiDaoDi )
	{
		m_isJinYuanZi = isJingYuanZi;
		m_nJinYuanZiCoin = nYuanZiCoin;
		m_isWaiBao = isWaiBao;
		m_isEnableShuangGang = isShuangGAng;
		m_isEnableYiZhanDaoDi = isYiDuiDaoDi;
	}

	void setKuaiChong(bool isKuaiChong, uint32_t nKuaiChongPool)
	{
		m_isKuaiChong = isKuaiChong;
		m_nKuaiChongPool = nKuaiChongPool;
	}

	void setHuaZaBiXiaHu(bool isHuaZa, bool isBiXiaHu,bool isSiLianFeng, bool isCircleType, bool isJieZhuangBi )
	{
		m_isBiXiaHu = isBiXiaHu;
		m_isHuaZa = isHuaZa;
		m_isSiLianFeng = isSiLianFeng;
		m_isCircleType = isCircleType;
		m_isJieZhuangBi = isJieZhuangBi;
	}

	std::shared_ptr<ISingleRoundRecorder> createSingleRoundRecorder()override
	{
		return std::make_shared<NJMJSingleRoundRecorder>();
	}

	void buildZhanJiMsg( Json::Value& jsMsg )override
	{
		IGameRoomRecorder::buildZhanJiMsg(jsMsg);

		Json::Value jsRoomOpts;

		jsRoomOpts["isJinYuanZi"] = m_isJinYuanZi ? 1 : 0 ;
		jsRoomOpts["yuanZi"] = m_nJinYuanZiCoin;

		jsRoomOpts["isKuaiChong"] = m_isKuaiChong ? 1 : 0;
		jsRoomOpts["kuaiChongPool"] = m_nKuaiChongPool;

		jsRoomOpts["isHuaZa"] = m_isHuaZa ? 1 : 0;
		jsRoomOpts["isBiXiaHu"] = m_isBiXiaHu ? 1 : 0;
		
		jsRoomOpts["isWaiBao"] = m_isWaiBao ? 1 : 0;

		jsRoomOpts["isSiLianFeng"] = m_isSiLianFeng ? 1 : 0;
		jsRoomOpts["isCircle"] = m_isCircleType ? 1 : 0;

		jsRoomOpts["isJieZhuangBi"] = m_isJieZhuangBi ? 1 : 0;
		jsRoomOpts["isShuangGang"] = m_isEnableShuangGang ? 1 : 0;
		jsRoomOpts["isYiDuiDaoDi"] = m_isEnableYiZhanDaoDi ? 1 : 0;

		jsMsg["roomOpts"] = jsRoomOpts;
	}

protected:
	void getRoomInfoOptsForSave(Json::Value& jsRoomOpts) override 
	{
		jsRoomOpts["isJingYuanZi"] = m_isJinYuanZi;
		jsRoomOpts["nYuanZiCoin"] = m_nJinYuanZiCoin;

		jsRoomOpts["isKuaiChong"] = m_isKuaiChong;
		jsRoomOpts["nKuaiChongPool"] = m_nKuaiChongPool;

		jsRoomOpts["isHuaZa"] = m_isHuaZa;
		jsRoomOpts["isBiXiaHu"] = m_isBiXiaHu;
		jsRoomOpts["isCircle"] = m_isCircleType;

		jsRoomOpts["isWaiBao"] = m_isWaiBao;

		jsRoomOpts["isSiLianFeng"] = m_isSiLianFeng;

		jsRoomOpts["isJieZhuangBi"] = m_isJieZhuangBi;

		jsRoomOpts["isShuangGang"] = m_isEnableShuangGang;
		jsRoomOpts["isYiDuiDaoDi"] = m_isEnableYiZhanDaoDi;
	};

    void restoreRoomInfoOpts(Json::Value& jsRoomOpts) override 
	{
		if (jsRoomOpts.isNull())
		{
			LOGFMTE("room opts is nullptr ");
			return;
		}

		m_isJinYuanZi = jsRoomOpts["isJingYuanZi"].asBool();
		m_nJinYuanZiCoin = jsRoomOpts["nYuanZiCoin"].asUInt();

		m_isKuaiChong = jsRoomOpts["isKuaiChong"].asBool();
		m_nKuaiChongPool = jsRoomOpts["nKuaiChongPool"].asUInt();

		m_isBiXiaHu = jsRoomOpts["isBiXiaHu"].asBool();
		m_isHuaZa = jsRoomOpts["isHuaZa"].asBool();

		m_isWaiBao = jsRoomOpts["isWaiBao"].asBool();
		m_isSiLianFeng = jsRoomOpts["isSiLianFeng"].asBool();

		if ( jsRoomOpts["isCircle"].isNull() == false )
		{
			m_isCircleType = jsRoomOpts["isCircle"].asBool();
		}

		if (jsRoomOpts["isJieZhuangBi"].isNull() == false)
		{
			m_isJieZhuangBi = jsRoomOpts["isCircle"].asBool();
		}

		// new
		if (jsRoomOpts["isShuangGang"].isNull() == false)
		{
			m_isEnableShuangGang = jsRoomOpts["isShuangGang"].asBool();
		}

		if (jsRoomOpts["isYiDuiDaoDi"].isNull() == false)
		{
			m_isEnableYiZhanDaoDi = jsRoomOpts["isYiDuiDaoDi"].asBool();
		}

	};
protected:
	bool m_isJinYuanZi;
	uint32_t m_nJinYuanZiCoin;

	bool m_isKuaiChong;
	uint32_t m_nKuaiChongPool;

	bool m_isHuaZa;
	bool m_isBiXiaHu;
	bool m_isWaiBao;
	bool m_isSiLianFeng;
	bool m_isCircleType;
	bool m_isJieZhuangBi;
	bool m_isEnableShuangGang;
	bool m_isEnableYiZhanDaoDi;
};