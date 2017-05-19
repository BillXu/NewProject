#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class JJQERoom
	:public IMJRoom
{
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	uint8_t getRoomType()override { return eRoom_MJ_QingEr; }
	bool isHaveLouHu()override { return false; };
	bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)override { return false; }
	IMJPlayer* doCreateMJPlayer()override;
	void startGame()override;
	void willStartGame()override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	void packStartGameMsg(Json::Value& jsMsg)override;
	bool isEnableChaoZhuang();
	void onLastChaoZhuangPlayerViewCard( uint8_t nCard , bool isHua );
	bool isAnyPlayerFlyUp();
	bool isAnyPlayerAutoBuHua();
	bool informPlayerFlyUp( uint8_t nPlayerIdx );
	bool onPlayerDoFlyUp( uint8_t nIdx , std::vector<uint8_t>& vFlyUpCard );
	void onDoAllPlayersAutoBuHua();
	void onDoPlayerBuHua(uint8_t nIdx , uint8_t nHuaCard );
	bool isCardJianPai( uint8_t nCheckCard );
	uint8_t getJianZhang();
	float getSingleChaoZhuangRate();
	float getDoubleChaoZhuangRate();
	uint16_t getQiHuNeed();
	uint16_t getTopHuLimit();
	void onGameDidEnd()override;
	void onGameEnd()override;
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard)override;
	IMJPoker* getMJPoker()override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
protected:
	CMJCard m_tPoker;
	uint16_t m_nQiHuNeed;
	uint16_t m_nTopLimit;
	uint8_t m_nJianZhang;
	uint8_t m_nChaoZhuangLevel;  // 0 means , not chao zhuang ;
	uint16_t m_nQingErHuCnt;
	bool m_isHave13Hu;

	uint8_t m_nLastHuIdx;

};