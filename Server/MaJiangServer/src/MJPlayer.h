#pragma once
#include "IMJPlayer.h"
class MJPlayer
	:public IMJPlayer
{
public:
	void init(stEnterRoomData* pData)override;
	void onComeBackRoom(stEnterRoomData* pData)override;
	void onWillStartGame()override;
	void onStartGame()override;
	void onGameDidEnd()override;
	void onGameEnd()override;
	uint8_t getIdx() final;
	void setIdx(uint8_t nIdx)  final;
	int32_t getCoin() final;
	void setCoin(int32_t nNewCoin)final;
	void addOffsetCoin(int32_t nOffset) override;
	int32_t getOffsetCoin() override;
	void setState(uint32_t eState) override;
	bool haveState(uint32_t eState) override;
	uint32_t getState() final;
	uint32_t getSessionID()final;
	uint32_t getUID()final;
	void signGangFlag()final;
	void clearGangFlag()final;
	bool haveGangFalg()final;
	void signDecareBuGangFlag()final;
	void clearDecareBuGangFlag()final;
	bool haveDecareBuGangFalg()final;
	int32_t onRecievedSupplyCoin(uint32_t nSupplyCoin)final;
	bool isRobot()final;
private:
	uint8_t m_nPlayerType;
	uint32_t m_eState;
	uint32_t m_nUserUID;
	uint32_t m_nSessioID;
	uint8_t m_nIdx;
	int32_t m_nOffset;
	uint32_t m_nCoin;
	bool m_isHaveGang;
	bool m_isDeclareBuGang;
};