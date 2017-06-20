#pragma once
#include "IMJRoom.h"
#include "CommonDefine.h"
#include "MJCard.h"
class GG23Room
	:public IMJRoom
{
public:
	bool init(IGameRoomManager* pRoomMgr, stBaseRoomConfig* pConfig, uint32_t nSeialNum, uint32_t nRoomID, Json::Value& vJsValue) override;
	uint8_t getRoomType()override { return eRoom_GG23; }
	bool isHaveLouHu()override { return true; };
	bool isAnyPlayerRobotGang(uint8_t nInvokeIdx, uint8_t nCard)override { return false; }
	IMJPlayer* doCreateMJPlayer()override;
	void startGame()override;
	void willStartGame()override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	void packStartGameMsg(Json::Value& jsMsg)override;
	bool isAnyPlayerFlyUp();
	bool informPlayerFlyUp(uint8_t nPlayerIdx);
	bool onPlayerDoFlyUp(uint8_t nIdx, std::vector<uint8_t>& vFlyUpCard);
	bool isCardJianPai(uint8_t nCheckCard);
	uint16_t getQiHuNeed();
	void onGameDidEnd()override;
	void onGameEnd()override;
	void onPlayerAnGang(uint8_t nIdx, uint8_t nCard)override;
	IMJPoker* getMJPoker()override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
	bool isGameOver()override;
	void onPlayerPeng(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void onPlayerMingGang(uint8_t nIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void visitPlayerInfoForRoomInfo(IMJPlayer* pPlayer, Json::Value& jsPlayerInfo)override;
protected:
	int8_t getChaoZhuangRate(bool isAChao, bool isBChao);
protected:
	CMJCard m_tPoker;
	uint16_t m_nQiHuNeed;
	uint16_t m_nTopLimit;

	uint8_t m_nLastHuIdx;

};