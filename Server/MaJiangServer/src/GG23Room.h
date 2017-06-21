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
	IMJPlayer* doCreateMJPlayer()override;
	void startGame()override;
	void willStartGame()override;
	void onPlayerHu(std::vector<uint8_t>& vHuIdx, uint8_t nCard, uint8_t nInvokeIdx)override;
	void getSubRoomInfo(Json::Value& jsSubInfo)override;
	void sendPlayersCardInfo(uint32_t nSessionID)override;
	bool isAnyPlayerFlyUp();
	bool informPlayerFlyUp(uint8_t nPlayerIdx);
	bool onPlayerDoFlyUp(uint8_t nIdx, std::vector<uint8_t>& vFlyUpCard);
	bool isCardJianPai(uint8_t nCheckCard);
	uint16_t getQiHuNeed();
	void onGameDidEnd()override;
	void onGameEnd()override;
	IMJPoker* getMJPoker()override;
	bool onPlayerApplyLeave(uint32_t nPlayerUID)override;
	std::shared_ptr<IGameRoomRecorder> createRoomRecorder()override;
	bool isGameOver()override;
protected:
	CMJCard m_tPoker;
	uint16_t m_nQiHuNeed;

	uint8_t m_nLastHuIdx;

};